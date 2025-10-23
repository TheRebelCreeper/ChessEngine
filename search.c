#include <stdio.h>
#include <string.h>

#include "search.h"

#include <assert.h>
#include <math.h>

#include "history.h"
#include "move.h"
#include "movegen.h"
#include "movepick.h"
#include "tt.h"
#include "util.h"

void report_search_info(SearchInfo *root_info, int score, unsigned int start, unsigned int finish)
{
    // After searching all possible moves, compile stats
    root_info->ms = finish - start;
    root_info->nps = (unsigned int) (1000 * root_info->nodes / (root_info->ms));
    root_info->best_score = score;

    int mated = 0;
    if (root_info->best_score > MAX_MATE_SCORE) {
        root_info->best_score = (MATE_SCORE - root_info->best_score) / 2 + 1;
        mated = 1;
    }
    else if (root_info->best_score < -MAX_MATE_SCORE) {
        root_info->best_score = (-MATE_SCORE - root_info->best_score) / 2;
        mated = 1;
    }

    printf("info depth %d ", root_info->depth);
    printf("score %s %d ", (mated) ? "mate" : "cp", root_info->best_score);
    printf("time %u ", root_info->ms);
    printf("nodes %llu ", root_info->nodes);
    printf("nps %u ", root_info->nps);
    printf("pv");
    for (int i = 0; i < root_info->pv_table_length[0]; i++) {
        if (i >= root_info->depth || root_info->pv_table[0][i] == 0)
            break;
        printf(" ");
        print_move((root_info->pv_table[0][i]));
    }
    printf("\n");
}

void check_time_left(SearchInfo *info)
{
    // Check if time up, or interrupt from GUI
    if (info->timeset == 1 && get_time_ms() > info->stoptime) {
        info->stopped = 1;
    }
    read_input(info);
}

inline int is_repetition(const GameState *pos)
{
    for (int i = 1; i < pos->half_move_clock; i++) {
        int index = history_index - i;
        if (index < 0)
            break;
        if (pos_history[index] == pos->key)
            return 1;
    }
    return 0; // Detects a single rep
}

int search(int alpha, int beta, int depth, GameState *pos, SearchInfo *info)
{
    assert(info->ply >= 0 && info->ply <= MAX_PLY);
    int ply = info->ply;
    int in_check = is_in_check(pos);
    int pv_node = beta - alpha > 1;
    int is_root = (ply == 0);
    int score;

    MoveList move_list;
    MoveList fail_low_quiets;
    clear_movelist(&fail_low_quiets);
    Move best_move = 0;
    int best_score = -INF;

    // Update node count
    info->nodes++;

    // Update time left
    if ((info->nodes & 2047) == 0) {
        check_time_left(info);
        // Ran out of time
        if (info->stopped)
            return 0;
    }

    // Search has exceeded max depth, return static eval
    if (ply >= MAX_PLY) {
        return evaluation(pos);
    }

    // Enter qsearch
    if (depth <= 0) {
        info->pv_table_length[ply] = 0;
        return qsearch(alpha, beta, pos, info);
    }

    // Search for draws and repetitions
    // Don't need to search for repetition if halfMoveClock is low
    if (!is_root && ((pos->half_move_clock > 4 && is_repetition(pos)) || pos->half_move_clock == 100 ||
                     insufficient_material(pos))) {
        // Cut the pv line if this is a draw
        info->pv_table_length[ply] = 0;
        return 0;
    }

    // tt_hit is boolean
    TTEntry tt_entry;
    int tt_hit = probe_tt(pos, &tt_entry, ply);

    // Cutoff when we find valid TT entry
    if (!pv_node && tt_hit && tt_entry.depth >= depth
        && (tt_entry.flag == TT_EXACT
            || (tt_entry.flag == TT_UPPER && tt_entry.score <= alpha)
            || (tt_entry.flag == TT_LOWER && tt_entry.score >= beta))) {
        return tt_entry.score;
    }

    //int static_eval = evaluation(pos);

    unsigned char tt_flag = TT_UPPER;
    int legal;
    int total_moves = generate_moves(pos, &move_list);
    score_moves(&move_list, pos, tt_entry.move, info);
    int move_count = 0;

    for (int i = 0; i < total_moves; i++) {
        pick_move(&move_list, i);
        Move current = move_list.move[i];
        GameState new_pos = play_move(pos, current, &legal);

        if (!legal)
            continue;

        move_count++;
        info->ply++;

        // Save the current move into history
        history_index++;
        pos_history[history_index] = new_pos.key;

        // Does this move give check
        //int gives_check = is_in_check(&new_pos);

        // PVS
        int new_depth = depth - 1;
        if (move_count == 1) {
            score = -search(-beta, -alpha, new_depth, &new_pos, info);
        }
        else {
            int r = 0.77 + log(move_count) * log(depth) / 2.36;
            if (move_count <= FULL_DEPTH_MOVES || depth <= 2)
                r = 0;
            int reduced = CLAMP(new_depth - r, 0, new_depth);

            score = -search(-alpha - 1, -alpha, reduced, &new_pos, info);
            if (score > alpha && reduced < new_depth) {
                score = -search(-alpha - 1, -alpha, new_depth, &new_pos, info);
            }
            if (score > alpha && score < beta) {
                score = -search(-beta, -alpha, new_depth, &new_pos, info);
            }
        }

        // Unmake move by removing current move from history
        info->ply--;
        history_index--;

        // Update bestMove whenever found so all-nodes can be stored in TT
        if (score > best_score) {
            best_score = score;

            if (score > alpha) {
                best_move = current;
                alpha = score;

                if (pv_node) {
                    info->pv_table[ply][ply] = current;
                    // Crazy memcpy which copies PV from lower depth to current depth
                    memcpy((info->pv_table[ply]) + ply + 1, (info->pv_table[ply + 1]) + ply + 1,
                           info->pv_table_length[ply + 1] * sizeof(Move));
                    info->pv_table_length[ply] = info->pv_table_length[ply + 1] + 1;
                }

                if (score >= beta) {
                    tt_flag = TT_LOWER;
                    break;
                }
                tt_flag = TT_EXACT;
            }
        }

        if (current != best_move && !is_noisy(current)) {
            fail_low_quiets.move[fail_low_quiets.next_open++] = current;
        }

        if (info->stopped)
            return 0;
    }

    // When no more legal moves, return either mate score or draw
    if (move_count == 0) {
        info->pv_table_length[ply] = 0;
        return in_check ? -MATE_SCORE + ply : 0;
    }

    // Update history score if not a capture and beta cutoff
    if (best_move && !is_noisy(best_move)) {
        int bonus = score_history(pos, best_move, depth);
        int penalty = -bonus;
        update_history(pos, best_move, bonus);

        for (int i = 0; i < fail_low_quiets.next_open; i++) {
            update_history(pos, fail_low_quiets.move[i], penalty);
        }
    }

    save_tt(pos, best_move, best_score, tt_flag, depth, ply);
    return best_score;
}

int qsearch(int alpha, int beta, GameState *pos, SearchInfo *info)
{
    assert(info->ply >= 0 && info->ply <= MAX_PLY);
    int ply = info->ply;
    int in_check = is_in_check(pos);

    // Update time left
    if ((info->nodes & 2047) == 0) {
        check_time_left(info);
        // Ran out of time
        if (info->stopped)
            return 0;
    }

    // Search has exceeded max depth, return static eval
    if (ply >= MAX_PLY) {
        return evaluation(pos);
    }

    int static_eval;
    if (in_check) {
        static_eval = -MATE_SCORE + ply;
    }
    else {
        static_eval = evaluation(pos);
        if (static_eval >= beta) {
            return static_eval;
        }

        if (static_eval > alpha) {
            alpha = static_eval;
        }
    }

    int best_score = static_eval;

    int legal;
    int move_count = 0;
    MoveList move_list;
    int total_moves = (!in_check) ? generate_moves_qsearch(pos, &move_list) : generate_moves(pos, &move_list);
    score_moves(&move_list, pos, 0, info);

    for (int i = 0; i < total_moves; i++) {
        pick_move(&move_list, i);
        Move current = move_list.move[i];

        // Prune captures with bad SEE when not in check
        if (!in_check && see(pos, GET_MOVE_DST(current)) < -105) {
            continue;
        }

        GameState new_pos = play_move(pos, current, &legal);
        if (!legal)
            continue;
        move_count++;

        info->ply++;
        int score = -qsearch(-beta, -alpha, &new_pos, info);
        info->ply--;

        if (score > best_score) {
            best_score = score;
            if (score > alpha) {
                alpha = score;

                if (score >= beta) {
                    break;
                }
            }
        }
    }

    // When no more legal moves, return mate score
    if (move_count == 0 && in_check) {
        return -MATE_SCORE + ply;
    }

    return best_score;
}

void search_root(GameState *pos, SearchInfo *root_info)
{
    int max_search_depth = root_info->depth;
    int alpha = -INF;
    int beta = INF;
    int score = -INF;
    Move best_move = 0;

    // Clear information for root_info. Will have to do this for ID upon each depth
    unsigned int start = get_time_ms();
    clear_history();
    root_info->nodes = 0ULL;
    root_info->ply = 0;
    memset(root_info->killer_moves, 0, sizeof(root_info->killer_moves));

    for (int ID = 1; ID <= max_search_depth; ID++) {
        memset(root_info->pv_table, 0, sizeof(root_info->pv_table));
        memset(root_info->pv_table_length, 0, sizeof(root_info->pv_table_length));
        root_info->depth = ID;

        int new_score = search(alpha, beta, ID, pos, root_info);
        Move new_move = root_info->pv_table[0][0];

        // If time is up, and we have completed at least depth 1 search, break out of loop
        if (!new_move || (root_info->stopped == 1 && ID > 1))
            break;

        score = new_score;
        best_move = root_info->pv_table[0][0];

        unsigned int finish = get_time_ms() + 1;
        report_search_info(root_info, score, start, finish);
    }
    // Print the best move
    printf("bestmove ");
    print_move(best_move);
    printf("\n");
}
