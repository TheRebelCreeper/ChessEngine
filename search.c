#include "search.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "history.h"
#include "move.h"
#include "movegen.h"
#include "movepick.h"
#include "tt.h"
#include "util.h"

void report_search_info(SearchInfo *root_info, int score, unsigned int start, unsigned int finish)
{
    if (!root_info)
        exit(EXIT_FAILURE);

    // After searching all possible moves, compile stats
    root_info->ms = finish - start + 1;
    root_info->nps = (unsigned int) (1000 * root_info->nodes / (root_info->ms));

    bool mated = false;
    if (score > MAX_MATE_SCORE) {
        score = (MATE_SCORE - score) / 2 + 1;
        mated = true;
    }
    else if (score < -MAX_MATE_SCORE) {
        score = (-MATE_SCORE - score) / 2;
        mated = true;
    }

    printf("info depth %d ", root_info->depth);
    printf("score %s %d ", (mated) ? "mate" : "cp", score);
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
    if (info->timeset && get_time_ms() > info->stoptime) {
        info->stopped = true;
    }
    read_input(info);
}

inline bool is_repetition(const GameState *pos)
{
    for (int i = 1; i < pos->half_move_clock; i++) {
        int index = repetition_index - i;
        if (index < 0)
            break;
        if (repetition_history[index] == pos->key)
            return true;
    }
    return false; // Detects a single rep
}

inline int calculate_reduction(Move m, int move_count, int depth, bool pv_node)
{
    int r = 0.77 + log(move_count) * log(depth) / 2.36;
    if (move_count <= FULL_DEPTH_MOVES || depth <= 2)
        r = 0;
    r += !pv_node;
    return r;
}

int search(int alpha, int beta, int depth, GameState *pos, SearchInfo *info, bool cut_node)
{
    assert(info->ply >= 0 && info->ply <= MAX_PLY);
    int ply = info->ply;
    bool in_check = is_in_check(pos);
    bool pv_node = beta - alpha > 1;
    bool is_root = (ply == 0);
    assert(!(pv_node && cut_node));

    MoveList move_list;
    MoveList fail_low_quiets;
    clear_movelist(&fail_low_quiets);
    Move best_move = 0;
    int best_score = -INF;
    int score;

    // Update node count
    info->nodes++;

    // Update time left
    if ((info->nodes & 2047) == 0) {
        check_time_left(info);
    }

    // Ran out of time
    if (info->stopped) {
        info->pv_table_length[ply] = 0;
        return 0;
    }

    // Search has exceeded max depth, return static eval
    if (ply >= MAX_PLY) {
        return evaluation(pos);
    }

    if (!is_root) {
        // Check extensions
        if (in_check)
            depth++;

        // Search for draws and repetitions
        // Don't need to search for repetition if halfMoveClock is low
        if ((pos->half_move_clock > 4 && is_repetition(pos)) || pos->half_move_clock == 100 ||
            insufficient_material(pos)) {
            // Cut the pv line if this is a draw
            info->pv_table_length[ply] = 0;
            return 0;
        }

        // Mate distance pruning
        alpha = MAX(alpha, -MATE_SCORE + ply);
        beta = MIN(beta, MATE_SCORE - ply - 1);
        if (alpha >= beta)
            return alpha;
    }

    // Enter qsearch
    if (depth <= 0) {
        info->pv_table_length[ply] = 0;
        return qsearch(alpha, beta, pos, info);
    }

    // tt_hit is boolean, if no entry found, move is set to 0
    TTEntry tt_entry;
    bool tt_hit = probe_tt(pos, &tt_entry, ply);

    // Cutoff when we find valid TT entry
    if (!pv_node && tt_hit && tt_entry.depth >= depth
        && (tt_entry.flag == TT_EXACT
            || (tt_entry.flag == TT_UPPER && tt_entry.score <= alpha)
            || (tt_entry.flag == TT_LOWER && tt_entry.score >= beta))) {
        return tt_entry.score;
    }

    // IIR
    if (depth >= MIN_IIR_DEPTH && (pv_node || cut_node) && !tt_entry.move) {
        --depth;
    }

    int static_eval = evaluation(pos);
    if (!pv_node && !in_check) {
        assert(!is_root);

        // Reverse Futility Pruning
        int rfp_margin = 75 * depth;
        if (depth <= 6 && static_eval - rfp_margin >= beta) {
            return static_eval;
        }

        // Razoring
        if (depth <= 4 && abs(alpha) < MATE_SCORE && static_eval + 250 * depth <= alpha) {
            int razor_score = qsearch(alpha, alpha + 1, pos, info);
            if (razor_score <= alpha)
                return razor_score;
        }

        // Null Move Pruning
        if (depth >= 4 && static_eval >= beta && info->move_stack[ply - 1] && !only_has_pawns(pos, pos->turn)) {
            int r = 4;

            // Make the null move
            GameState new_pos;
            make_null_move(pos, &new_pos);
            repetition_history[++repetition_index] = new_pos.key;

            // Save null move to move_stack
            info->move_stack[ply] = 0;
            info->ply++;
            int null_score = -search(-beta, -beta + 1, depth - r, &new_pos, info, !cut_node);
            info->ply--;
            repetition_index--;

            if (null_score >= beta && null_score < MATE_SCORE)
                return null_score;
        }
    }

    unsigned char tt_flag = TT_UPPER;
    int total_moves = generate_moves(pos, &move_list);
    score_moves(&move_list, pos, tt_entry.move, ply);
    int move_count = 0;

    for (int i = 0; i < total_moves; i++) {
        pick_move(&move_list, i);
        Move current = move_list.move[i];
        GameState new_pos;

        // Make move and skip if illegal
        if (!make_move(pos, &new_pos, current))
            continue;

        bool noisy = is_noisy(current);
        if (best_score > -MATE_SCORE && !in_check) {
            // Futility Pruning
            if (!noisy && depth <= 8 && abs(alpha) < MATE_SCORE && static_eval + depth * 125 <= alpha) {
                continue;
            }

            // SEE pruning
            int see_threshold = noisy ? -75 * depth : -25 * depth;
            if (see(pos, GET_MOVE_DST(current)) <= see_threshold)
                continue;
        }

        move_count++;
        info->ply++;

        // Save current move to move_stack
        info->move_stack[ply] = current;

        // Save the current move into history
        repetition_history[++repetition_index] = new_pos.key;

        // PVS
        int new_depth = depth - 1;
        if (move_count == 1) {
            score = -search(-beta, -alpha, new_depth, &new_pos, info, false);
        }
        else {
            int r = calculate_reduction(current, move_count, depth, pv_node);
            int reduced = CLAMP(new_depth - r, 0, new_depth);

            score = -search(-alpha - 1, -alpha, reduced, &new_pos, info, true);
            if (score > alpha && reduced < new_depth) {
                score = -search(-alpha - 1, -alpha, new_depth, &new_pos, info, !cut_node);
            }
            if (score > alpha && score < beta) {
                score = -search(-beta, -alpha, new_depth, &new_pos, info, false);
            }
        }

        // Unmake move by removing current move from history
        info->ply--;
        repetition_index--;

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

        if (current != best_move && !noisy) {
            fail_low_quiets.move[fail_low_quiets.next_open++] = current;
        }

        if (info->stopped) {
            info->pv_table_length[ply] = 0;
            return 0;
        }
    }

    // When no more legal moves, return either mate score or draw
    if (move_count == 0) {
        info->pv_table_length[ply] = 0;
        return in_check ? -MATE_SCORE + ply : 0;
    }

    // Update history score if not a capture and beta cutoff
    if (best_move && !is_noisy(best_move)) {
        push_killer_move(best_move, ply);

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
    bool in_check = is_in_check(pos);

    // Update time left
    if ((info->nodes & 2047) == 0) {
        check_time_left(info);
    }

    if (info->stopped)
        return 0;

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

    int move_count = 0;
    MoveList move_list;
    int total_moves = (!in_check) ? generate_moves_qsearch(pos, &move_list) : generate_moves(pos, &move_list);
    score_moves(&move_list, pos, 0, ply);

    for (int i = 0; i < total_moves; i++) {
        pick_move(&move_list, i);
        Move current = move_list.move[i];

        // Prune captures with bad SEE when not in check
        if (!in_check && see(pos, GET_MOVE_DST(current)) < -105) {
            continue;
        }

        GameState new_pos;
        if (!make_move(pos, &new_pos, current))
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
    int delta = INITIAL_ASP_WINDOW;
    int score = -INF;
    Move best_move = 0;

    // Clear information for root_info. Will have to do this for ID upon each depth
    unsigned int start = get_time_ms();
    clear_history();
    root_info->nodes = 0ULL;
    root_info->ply = 0;
    root_info->stopped = 0;
    memset(root_info->move_stack, 0, sizeof(root_info->move_stack));

    for (int iterative_depth = 1; iterative_depth <= max_search_depth; iterative_depth++) {
        memset(root_info->pv_table, 0, sizeof(root_info->pv_table));
        memset(root_info->pv_table_length, 0, sizeof(root_info->pv_table_length));
        root_info->depth = iterative_depth;

        int new_score = search(alpha, beta, iterative_depth, pos, root_info, false);

        if (new_score <= alpha || new_score >= beta) {
            alpha = -INF;
            beta = INF;
            iterative_depth -= 1;
            continue;
        }
        alpha = new_score - delta;
        beta = new_score + delta;

        // If time is up, and we have completed at least depth 1 search, break out of loop
        if (!root_info->pv_table_length[0] || (root_info->stopped && iterative_depth > 1))
            break;

        score = new_score;
        best_move = root_info->pv_table[0][0];

        unsigned int finish = get_time_ms();
        report_search_info(root_info, score, start, finish);
    }
    // Print the best move
    printf("bestmove ");
    print_move(best_move);
    printf("\n");
}
