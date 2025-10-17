#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "search.h"
#include "move.h"
#include "movegen.h"
#include "tt.h"
#include "util.h"

int following_pv = 0;

void check_time_left(SearchInfo *info)
{
    // Check if time up, or interrupt from GUI
    if (info->timeset == 1 && get_time_ms() > info->stoptime) {
        info->stopped = 1;
    }
    read_input(info);
}

void score_moves(MoveList *moves, GameState *pos, Move tt_move, SearchInfo *info)
{
    for (int i = 0; i < moves->next_open; i++) {
        // Score TT hits
        if (moves->list[i] == tt_move) {
            moves->score[i] = TT_HIT_SCORE;
            continue;
        }

        // Score captures
        if (moves->list[i] & IS_CAPTURE) {
            int piece_offset = 6 * pos->turn;
            moves->score[i] = MVV_LVA_TABLE[GET_MOVE_PIECE(moves->list[i]) - piece_offset][
                                  GET_MOVE_CAPTURED(moves->list[i])] + KILLER_ONE;

            // Give bad score to results with negative SEE
            if (see(pos, GET_MOVE_DST(moves->list[i])) < -100) {
                moves->score[i] -= KILLER_ONE;
            }
        }
        // Score quiet moves
        else {
            // Check First Killer Move
            if (moves->list[i] == info->killer_moves[0][info->ply]) {
                moves->score[i] = KILLER_ONE;
            }
            // Check Second Killer Move
            else if (moves->list[i] == info->killer_moves[1][info->ply]) {
                moves->score[i] = KILLER_TWO;
            }
            else {
                moves->score[i] = MIN(
                    info->history[pos->turn][GET_MOVE_SRC(moves->list[i])][GET_MOVE_DST(moves->list[i])] +
                    HISTORY_SCORE_MIN,
                    HISTORY_SCORE_MAX);
            }
        }
    }
}

void pick_move(MoveList *moves, int start_index)
{
    int best_index = start_index;
    for (int i = start_index; i < moves->next_open; i++) {
        if (moves->score[i] > moves->score[best_index])
            best_index = i;
    }

    Move tmp = moves->list[start_index];
    int temp_score = moves->score[start_index];
    moves->list[start_index] = moves->list[best_index];
    moves->score[start_index] = moves->score[best_index];
    moves->list[best_index] = tmp;
    moves->score[best_index] = temp_score;
}

inline int is_repetition(GameState *pos)
{
    int reps = 0;
    for (int i = 1; i < pos->half_move_clock; i++) {
        int index = history_index - i;
        if (index < 0)
            break;
        if (pos_history[index] == pos->key)
            reps++;
    }
    //return reps >= 2; //This handles true threefold instead of single rep
    return reps != 0; // Detects a single rep
}

inline int is_tactical(Move move, int in_check, int gives_check)
{
    return GET_MOVE_CAPTURED(move) != NO_CAPTURE || GET_MOVE_PROMOTION(move) != NO_PROMOTION || in_check || gives_check;
}

inline int ok_to_reduce(Move move, int in_check, int gives_check, int pv)
{
    return !is_tactical(move, in_check, gives_check) && !pv;
}

int search(int alpha, int beta, int depth, GameState *pos, SearchInfo *info, int prune_null)
{
    char node_bound = TT_ALL;
    int size, legal, move_count = 0;
    int ply = info->ply;
    int in_check = is_in_check(pos);
    int is_pv_node = beta - alpha > 1;
    int is_root = (ply == 0);
    int enable_futility_pruning = 0;
    int static_eval = evaluation(pos);

    MoveList move_list;
    Move best_move = 0;
    int best_score = -INF;

    info->nodes++;

    // Search has exceeded max depth, return static eval
    if (ply >= MAX_PLY) {
        return static_eval;
    }

    // Update time left
    if ((info->nodes & 2047) == 0) {
        check_time_left(info);
        // Ran out of time
        if (info->stopped)
            return alpha;
    }

    // Increase depth if currently in check since there are few replies
    if (in_check)
        depth++;

    // Enter qsearch if not in check
    if (depth <= 0) {
        info->pv_table_length[ply] = 0;
        info->nodes--;
        return qsearch(alpha, beta, depth, pos, info);
    }

    // Mate distance pruning
    // Stops from searching for mate when faster mate already found
    if (!is_root) {
        int distance = INF - ply;
        if (distance < beta) {
            beta = distance;
            if (alpha >= distance)
                return distance;
        }

        distance = -INF + ply;
        if (distance > alpha) {
            alpha = distance;
            if (beta <= distance)
                return distance;
        }
    }

    // Search for draws and repetitions
    // Don't need to search for repetition if halfMoveClock is low
    if (!is_root && ((pos->half_move_clock > 4 && is_repetition(pos)) || pos->half_move_clock == 100 ||
                     insufficient_material(pos))) {
        // Cut the pv line if this is a draw
        info->pv_table_length[ply] = 0;
        return 0;
    }

    // Check hash table for best move
    // Do not cut if pv node
    Move ttMove = 0;
    int score = probe_tt(pos, &ttMove, alpha, beta, depth, ply);
    if (score != INVALID_SCORE && !is_root && !is_pv_node) {
        return score;
    }

    // Reverse Futility Pruning
    if (depth < 4 && !is_pv_node && !in_check && abs(beta) < CHECKMATE) {
        int rfp_margin = 80 * depth;
        if (static_eval - rfp_margin >= beta)
            return beta;
    }

    // Razoring
    // If static eval is a good amount below alpha, we are probably at an all-node.
    // Do a qsearch just to confirm. If the qsearch fails high, a capture gained back
    // the material and trust its result since a quiet move probably can't gain
    // as much.
    if (!is_pv_node && !in_check
        && abs(alpha) < CHECKMATE
        && depth <= 3 && static_eval <= alpha - RAZOR_MARGIN[depth]) {
        if (depth == 1)
            return qsearch(alpha, beta, 0, pos, info);

        int r_window = alpha - RAZOR_MARGIN[depth];
        int value = qsearch(r_window, r_window + 1, 0, pos, info);
        // Fail hard here to be safe
        if (value <= r_window)
            return value;
    }

    // Null move pruning
    if (prune_null && static_eval >= beta && !is_pv_node && !in_check && depth >= 3 && !
        only_has_pawns(pos, pos->turn)) {
        // Reduce by either 3 or 4 ply depending on depth
        int r = (depth <= 6) ? 3 : 4;

        // Make the null move
        GameState new_pos;
        memcpy(&new_pos, pos, sizeof(GameState));
        new_pos.turn ^= 1;
        new_pos.enpassant_square = none;
        new_pos.key ^= side_key;
        if (pos->enpassant_square != none)
            new_pos.key ^= epKey[pos->enpassant_square & 7];
        info->ply++;

        // Search resulting position with reduced depth
        score = -search(-beta, -beta + 1, depth - 1 - r, &new_pos, info, 0);

        // Unmake null move
        info->ply--;

        // Ran out of time
        if (info->stopped)
            return alpha;

        if (score >= beta && abs(score) < CHECKMATE)
            return beta;
    }

    // Check if move is eligible for futility pruning
    if (depth < 9 && !is_pv_node && !in_check && alpha < CHECKMATE) {
        if (static_eval + futility_margins[depth] <= alpha)
            enable_futility_pruning = 1;
    }

    move_list = generate_moves(pos, &size);
    score_moves(&move_list, pos, ttMove, info);

    for (int i = 0; i < size; i++) {
        pick_move(&move_list, i);
        Move current = move_list.list[i];
        GameState new_pos = play_move(pos, current, &legal);

        // Increment history index to next depth

        if (!legal)
            continue;

        move_count++;
        info->ply++;

        // Save the current move into history
        history_index++;
        pos_history[history_index] = new_pos.key;

        // Does this move give check
        int gives_check = is_in_check(&new_pos);

        // Futility Pruning
        if (enable_futility_pruning && move_count > 1) {
            if (!is_tactical(current, in_check, gives_check)) {
                info->ply--;
                history_index--;
                continue;
            }
        }

        // LMR psuedocode comes from https://web.archive.org/web/20150212051846/http://www.glaurungchess.com/lmr.html
        // via Tord Romstad

        // Do a full depth search on PV
        if (move_count == 1) {
            score = -search(-beta, -alpha, depth - 1, &new_pos, info, 1);
        }
        // LMR on non PV node
        else {
            int r = 0.99 + log(depth) * log(move_count) / 3.14;
            if (move_count > FULL_DEPTH_MOVES && (depth - r - 1 > 0) && ok_to_reduce(
                    current, in_check, gives_check, is_pv_node)) {
                // Reduced search without null moves
                score = -search(-alpha - 1, -alpha, depth - r - 1, &new_pos, info, 1);
            }
            else {
                score = alpha + 1;
            }

            if (score > alpha) {
                score = -search(-alpha - 1, -alpha, depth - 1, &new_pos, info, 1);
                if ((score > alpha) && (score < beta)) {
                    score = -search(-beta, -alpha, depth - 1, &new_pos, info, 1);
                }
            }
        }

        // Update bestMove whenever found so all-nodes can be stored in TT
        if (score > best_score) {
            best_score = score;
            best_move = current;
        }

        // Unmake move by removing current move from history
        info->ply--;
        history_index--;

        if (info->stopped)
            return alpha;

        if (score >= beta) {
            save_tt(pos, current, beta, TT_CUT, depth, ply);
            // If the move is not a capture, save as killer move
            if ((current & IS_CAPTURE) == 0) {
                if (current != info->killer_moves[0][ply]) {
                    info->killer_moves[1][ply] = info->killer_moves[0][ply];
                    info->killer_moves[0][ply] = current;
                }
                info->history[pos->turn][GET_MOVE_SRC(current)][GET_MOVE_DST(current)] += (depth * depth);
                if (info->history[pos->turn][GET_MOVE_SRC(current)][GET_MOVE_DST(current)] > HISTORY_SCORE_MAX) {
                    info->history[pos->turn][GET_MOVE_SRC(current)][GET_MOVE_DST(current)] >>= 1;
                }
            }
            return beta;
        }

        if (score > alpha) {
            node_bound = TT_PV;
            info->pv_table[ply][ply] = current;
            // Crazy memcpy which copies PV from lower depth to current depth
            memcpy((info->pv_table[ply]) + ply + 1, (info->pv_table[ply + 1]) + ply + 1,
                   info->pv_table_length[ply + 1] * sizeof(Move));
            info->pv_table_length[ply] = info->pv_table_length[ply + 1] + 1;
            alpha = score;
        }
    }

    // If only one legal move make the move right away
    if (move_count == 1 && is_root && info->timeset == 1 && depth > 2) {
        info->stopped = 1;
    }

    if (move_count == 0) {
        // When no more legal moves, the pv does not exist
        info->pv_table_length[ply] = 0;
        if (in_check) {
            return -INF + ply;
        }
        return 0;
    }

    save_tt(pos, best_move, alpha, node_bound, depth, ply);
    return alpha;
}

// Cannot enter while in check initially
int qsearch(int alpha, int beta, int depth, GameState *pos, SearchInfo *info)
{
    MoveList move_list;
    int size, legal, move_count = 0;
    int in_check = is_in_check(pos);

    int static_eval = evaluation(pos);
    int bestScore = static_eval;
    info->nodes++;

    if (info->ply >= MAX_PLY) {
        return static_eval;
    }

    // Check if time is up every 2048 nodes
    if ((info->nodes & 2047) == 0) {
        check_time_left(info);
        if (info->stopped)
            return static_eval;
    }

    if (static_eval >= beta && !in_check) {
        return static_eval;
    }

    if (static_eval > alpha && !in_check) {
        alpha = static_eval;
    }

    move_list = generate_moves(pos, &size);
    score_moves(&move_list, pos, 0, info);

    // TODO: use moves that give check within the first two ply of qsearch
    for (int i = 0; i < size; i++) {
        pick_move(&move_list, i);
        Move current = move_list.list[i];

        if (!in_check && GET_MOVE_CAPTURED(current) == NO_CAPTURE && GET_MOVE_PROMOTION(current) == NO_PROMOTION) {
            continue;
        }

        // Prune captures with bad SEE when not in check
        if (!in_check && see(pos, GET_MOVE_DST(current)) < 0) {
            continue;
        }

        GameState new_pos = play_move(pos, current, &legal);
        if (!legal)
            continue;
        move_count++;

        info->ply++;
        int score = -qsearch(-beta, -alpha, depth, &new_pos, info);
        info->ply--;

        if (info->stopped)
            return alpha;

        if (score > bestScore) {
            bestScore = score;
        }

        if (score >= beta) {
            return score;
        }

        if (score > alpha) {
            alpha = score;
        }
    }

    // If no more legal moves and we are in check checkmate
    if (move_count == 0 && in_check) {
        return -INF + info->ply;
    }

    // Draw by 50 move rule
    if (pos->half_move_clock == 100 || insufficient_material(pos)) {
        return 0;
    }

    return bestScore;
}

void search_root(GameState *pos, SearchInfo *root_info)
{
    int score, best_score;
    Move best_move = 0;
    int alpha = -INF;
    int beta = INF;
    int search_depth = root_info->depth;
    unsigned int start, finish;

    // Clear information for root_info. Will have to do this for ID upon each depth
    start = get_time_ms();
    root_info->nodes = 0ULL;
    root_info->ply = 0;
    memset(root_info->killer_moves, 0, sizeof(root_info->killer_moves));
    memset(root_info->history, 0, sizeof(root_info->history));

    for (int ID = 1; ID <= search_depth; ID++) {
        memset(root_info->pv_table, 0, sizeof(root_info->pv_table));
        memset(root_info->pv_table_length, 0, sizeof(root_info->pv_table_length));
        root_info->depth = ID;
        following_pv = 1;

        score = search(alpha, beta, ID, pos, root_info, 1);

        if (root_info->stopped == 1 && ID > 1)
            break;
        best_score = score;

#ifdef ASPIRATION_WINDOW
        if (best_score <= alpha || best_score >= beta) {
            alpha = -INF;
            beta = INF;
            ID--;
            continue;
        }

        alpha = MAX(best_score - 50, -INF);
        beta = MIN(best_score + 50, INF);
#endif

        best_move = root_info->pv_table[0][0];

        // After searching all possible moves, compile stats
        finish = get_time_ms() + 1;
        root_info->ms = finish - start;
        root_info->nps = (unsigned int) (1000 * root_info->nodes / (root_info->ms));
        root_info->best_score = best_score;

        int mated = 0;
        if (root_info->best_score > CHECKMATE) {
            root_info->best_score = (INF - root_info->best_score) / 2 + 1;
            mated = 1;
        }
        else if (root_info->best_score < -CHECKMATE) {
            root_info->best_score = (-INF - root_info->best_score) / 2;
            mated = 1;
        }

        printf("info depth %d ", ID);
        printf("score %s %d ", (mated) ? "mate" : "cp", root_info->best_score);
        printf("time %u ", root_info->ms);
        printf("nodes %llu ", root_info->nodes);
        printf("nps %u ", root_info->nps);
        printf("pv");
        for (int i = 0; i < root_info->pv_table_length[0]; i++) {
            if (i >= ID || root_info->pv_table[0][i] == 0)
                break;
            printf(" ");
            print_move((root_info->pv_table[0][i]));
        }
        printf("\n");
    }
    // Print the best move
    printf("bestmove ");
    print_move(best_move);
    printf("\n");
}
