#include "search.h"
#include <assert.h>
#include <inttypes.h>
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

static int lmr_table[MAX_PLY][MAX_MOVES];
static int lmp_table[16][2];

void report_search_info(SearchInfo *root_info, int score)
{
    if (!root_info)
        exit(EXIT_FAILURE);

    // After searching all possible moves, compile stats
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
    printf("nodes %"PRIu64" ", root_info->nodes);
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

void init_lmr_table()
{
    for (int depth = 0; depth < MAX_PLY; depth++) {
        for (int move_count = 0; move_count < MAX_MOVES; move_count++) {
            lmr_table[depth][move_count] = 0.77 + log(move_count) * log(depth) / 2.36;
        }
    }
}

void init_lmp_table()
{
    // Capped at 15 depth since cannot have more than 256 possible legal moves
    for (int depth = 0; depth < 16; depth++) {
        for (int imp = 0; imp < 2; imp++) {
            lmp_table[depth][imp] = (MIN_LMP_MOVES + depth * depth) / (2 - imp);
        }
    }
}

int calculate_reduction(Move m, int move_count, int depth)
{
    // Prevent out of bounds error when approaching max ply
    depth = MIN(depth, MAX_PLY - 1);
    int r = lmr_table[depth][move_count];
    if (move_count <= MIN_LMR_MOVES || depth <= MIN_LMR_DEPTH)
        r = 0;
    return r;
}

inline bool calculate_improving(SearchInfo *info, int static_eval, bool in_check)
{
    int ply = info->ply;
    if (in_check) {
        return false;
    }
    else if (ply > 1 && info->static_eval_stack[ply - 2] != -INF) {
        return static_eval > info->static_eval_stack[ply - 2];
    }
    else if (ply > 3 && info->static_eval_stack[ply - 4] != -INF) {
        return static_eval > info->static_eval_stack[ply - 4];
    }
    return true;
}

int search(int alpha, int beta, int depth, GameState *pos, SearchInfo *info, bool cut_node)
{
    assert(info->ply >= 0 && info->ply <= MAX_PLY);
    int ply = info->ply;
    bool in_check = is_in_check(pos);
    bool pv_node = beta - alpha > 1;
    bool is_root = (ply == 0);
    assert(!(pv_node && cut_node));
    assert(!is_root || pv_node);

    Move excluded = info->excluded_stack[ply];       

    assert(!is_root || excluded == 0);

    MoveList move_list;
    MoveList fail_low_quiets;
    clear_movelist(&fail_low_quiets);
    Move best_move = 0;
    int best_score = -INF;
    int score = -INF;

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
        // Check extension
        if (in_check)
            depth++;

        // 50mr and insufficient material
        if (pos->half_move_clock >= 100 || insufficient_material(pos)) {
            info->pv_table_length[ply] = 0;
            return 0;
        }

        // Don't need to search for repetition if halfMoveClock is low
        if (pos->half_move_clock > 4 && is_repetition(pos)) {
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
        return qsearch(alpha, beta, pos, info, pv_node);
    }

    // tt_hit is boolean, if no entry found, move is set to 0
    TTEntry tt_entry;
    tt_entry.move = 0;
    bool tt_hit = false;
    if (!excluded) {
        tt_hit = probe_tt(pos, &tt_entry, ply);

        // Cutoff when we find valid TT entry
        if (!pv_node && tt_hit && tt_entry.depth >= depth
            && (tt_entry.flag == TT_EXACT
                || (tt_entry.flag == TT_UPPER && tt_entry.score <= alpha)
                || (tt_entry.flag == TT_LOWER && tt_entry.score >= beta))) {
            return tt_entry.score;
        }
    }

    // IIR
    if (depth >= MIN_IIR_DEPTH && !excluded && (pv_node || cut_node) && !tt_entry.move) {
        depth--;
    }

    int static_eval = -INF, static_eval_raw = -INF;
    if (!excluded) {
        if (in_check) {
            static_eval_raw = -INF;
            static_eval = -INF;
        }
        else {
            if (tt_hit && tt_entry.flag != TT_NONE && tt_entry.static_eval != -INF) {
                static_eval_raw = tt_entry.static_eval;
            }
            else {
                static_eval_raw = evaluation(pos);
            }
            static_eval = correct_static_eval(pos, static_eval_raw);
        }
        info->static_eval_stack[ply] = static_eval;
    }

    bool improving = calculate_improving(info, static_eval, in_check);
    if (!pv_node && !in_check && !excluded) {
        assert(!is_root);

        // Reverse Futility Pruning
        int rfp_margin = 75 * MAX(depth - improving, 0);
        if (depth <= 6 && static_eval - rfp_margin >= beta)
            return static_eval;

        // Razoring
        if (depth <= 4 && abs(alpha) < MAX_MATE_SCORE && static_eval + 250 * depth <= alpha) {
            int razor_score = qsearch(alpha, alpha + 1, pos, info, pv_node);
            if (razor_score <= alpha)
                return razor_score;
        }

        // Null Move Pruning
        if (depth >= MIN_NMP_DEPTH && static_eval >= beta && info->move_stack[ply - 1]
            && !only_has_pawns(pos, pos->turn)) {
            int r = 4 + depth / 4;

            // Make the null move
            GameState null_pos;
            make_null_move(pos, &null_pos);
            repetition_history[++repetition_index] = null_pos.key;
            info->move_stack[ply] = 0;

            info->ply++;
            prefetch_tt(null_pos.key);
            int null_score = -search(-beta, -beta + 1, depth - r, &null_pos, info, !cut_node);
            info->ply--;
            repetition_index--;

            if (null_score >= beta && null_score < MATE_SCORE)
                return null_score;
        }
    }

    u8 tt_flag = TT_UPPER;
    int total_moves = generate_moves(pos, &move_list);
    score_moves(&move_list, pos, tt_entry.move, ply);
    int move_count = 0;

    for (int i = 0; i < total_moves; i++) {
        pick_move(&move_list, i);
        Move current = move_list.move[i];

        if (current == excluded)
            continue;

        bool noisy = is_noisy(current);
        if (best_score > -MATE_SCORE && !in_check) {
            // Futility Pruning
            if (!noisy && depth <= 8 && abs(alpha) < MAX_MATE_SCORE && static_eval + depth * 125 <= alpha) {
                continue;
            }

            // Late Move Pruning
            if (!noisy && move_count >= lmp_table[MIN(depth, 15)][improving]) {
                continue;
            }

            // SEE pruning
            int see_threshold = noisy ? -75 * depth : -25 * depth;
            if (see(pos, GET_MOVE_DST(current)) <= see_threshold)
                continue;
        }

        // Make move and skip if illegal
        GameState new_pos;
        if (!make_move(pos, &new_pos, current))
            continue;

        move_count++;
        info->ply++;

        int extension = 0;
        if (!is_root && depth >= 8 && current == tt_entry.move && !excluded && tt_entry.depth >= depth
            - 4 && tt_entry.flag != TT_UPPER) {
            int s_beta = MAX(-INF + 1, tt_entry.score - 2 * depth);
            int s_depth = (depth - 1) / 2;
            info->excluded_stack[ply] = current;
            int s_score = search(s_beta - 1, s_beta, s_depth, pos, info, cut_node);
            info->excluded_stack[ply] = 0;

            if (s_score < s_beta)
                extension = 1;
        }

        prefetch_tt(new_pos.key);

        // Save current move to move_stack
        info->move_stack[ply] = current;
        info->static_eval_stack[ply] = static_eval;

        // Save the current move into history
        repetition_history[++repetition_index] = new_pos.key;

        // PVS
        bool gives_check = is_in_check(&new_pos);
        int new_depth = depth + extension - 1;
        // Start with Late Move Reduction
        if (move_count >= MIN_LMR_MOVES && depth >= MIN_LMR_DEPTH) {
            int r = calculate_reduction(current, move_count, depth);
            r += !pv_node;
            r -= gives_check;
            r -= improving;
            int reduced = CLAMP(new_depth - r, 1, new_depth);
            score = -search(-alpha - 1, -alpha, reduced, &new_pos, info, true);
            if (score > alpha && reduced < new_depth) {
                score = -search(-alpha - 1, -alpha, new_depth, &new_pos, info, !cut_node);
            }
        }
        // Unreduced zero window to try and raise alpha
        else if (!pv_node || move_count > 1) {
            score = -search(-alpha - 1, -alpha, new_depth, &new_pos, info, !cut_node);
        }
        // Full window search
        if (pv_node && (move_count == 1 || score > alpha)) {
            score = -search(-beta, -alpha, new_depth, &new_pos, info, false);
        }

        // Unmake move by removing current move from history
        info->ply--;
        repetition_index--;

        if (info->stopped) {
            return 0;
        }

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

    if (!excluded)
        save_tt(pos, best_move, static_eval_raw, best_score, tt_flag, depth, ply);
    return best_score;
}

int qsearch(int alpha, int beta, GameState *pos, SearchInfo *info, bool pv_node)
{
    assert(info->ply >= 0 && info->ply <= MAX_PLY);
    int ply = info->ply;
    bool in_check = is_in_check(pos);

    // Update time left
    if ((info->nodes & 2047) == 0) {
        check_time_left(info);
    }

    // Ran out of time
    if (info->stopped)
        return 0;

    // Search has exceeded max depth, return static eval
    if (ply >= MAX_PLY) {
        return evaluation(pos);
    }

    // 50mr and insufficient material
    if (pos->half_move_clock >= 100 || insufficient_material(pos)) {
        info->pv_table_length[ply] = 0;
        return 0;
    }

    // Don't need to search for repetition if halfMoveClock is low
    if (pos->half_move_clock > 4 && is_repetition(pos)) {
        info->pv_table_length[ply] = 0;
        return 0;
    }

    // tt_hit is boolean, if no entry found, move is set to 0
    TTEntry tt_entry;
    bool tt_hit = probe_tt(pos, &tt_entry, ply);
    // Cutoff when we find valid TT entry
    if (!pv_node && tt_hit
        && (tt_entry.flag == TT_EXACT
            || (tt_entry.flag == TT_UPPER && tt_entry.score <= alpha)
            || (tt_entry.flag == TT_LOWER && tt_entry.score >= beta))) {
        return tt_entry.score;
    }

    int static_eval, static_eval_raw;
    if (in_check) {
        static_eval_raw = -INF;
        static_eval = -MATE_SCORE + ply;
    }
    else {
        if (tt_hit && tt_entry.flag != TT_NONE && tt_entry.static_eval != -INF) {
            static_eval_raw = tt_entry.static_eval;
        }
        else {
            static_eval_raw = evaluation(pos);
        }
        static_eval = correct_static_eval(pos, static_eval_raw);
        if (static_eval >= beta) {
            return static_eval;
        }

        if (static_eval > alpha) {
            alpha = static_eval;
        }
    }

    int best_score = static_eval;
    Move best_move = 0;

    u8 tt_flag = TT_UPPER;
    int move_count = 0;
    MoveList move_list;
    int total_moves = (!in_check) ? generate_moves_qsearch(pos, &move_list) : generate_moves(pos, &move_list);
    score_moves(&move_list, pos, tt_entry.move, ply);

    for (int i = 0; i < total_moves; i++) {
        pick_move(&move_list, i);
        Move current = move_list.move[i];

        // Prune captures with bad SEE when not in check
        if (!in_check && see(pos, GET_MOVE_DST(current)) < -100) {
            continue;
        }

        // Make move and skip if illegal
        GameState new_pos;
        if (!make_move(pos, &new_pos, current))
            continue;

        move_count++;
        info->ply++;
        prefetch_tt(new_pos.key);
        int score = -qsearch(-beta, -alpha, &new_pos, info, pv_node);
        info->ply--;

        if (info->stopped) {
            return 0;
        }

        if (score > best_score) {
            best_score = score;
        }
        if (score > alpha) {
            alpha = score;
            best_move = current;
            tt_flag = TT_EXACT;
        }
        if (score >= beta) {
            tt_flag = TT_LOWER;
            break;
        }
    }

    // When no more legal moves, return mate score
    if (move_count == 0 && in_check) {
        return -MATE_SCORE + ply;
    }

    save_tt(pos, best_move, static_eval_raw, best_score, tt_flag, 0, ply);
    return best_score;
}

void search_root(GameState *pos, SearchInfo *search_info)
{
    int max_search_depth = search_info->depth;
    int alpha = -INF;
    int beta = INF;
    int delta = 0;
    int score = -INF;
    Move best_move = 0;

    // Clear information for search_info. Will have to do this for ID upon each depth
    unsigned int start = get_time_ms();
    clear_history();
    search_info->nodes = 0ULL;
    search_info->ply = 0;
    search_info->stopped = false;
    memset(search_info->move_stack, 0, sizeof(search_info->move_stack));
    memset(search_info->excluded_stack, 0, sizeof(search_info->excluded_stack));
    memset(search_info->static_eval_stack, -INF, sizeof(search_info->static_eval_stack));

    for (int iterative_depth = 1; iterative_depth <= max_search_depth; iterative_depth++) {
        memset(search_info->pv_table, 0, sizeof(search_info->pv_table));
        memset(search_info->pv_table_length, 0, sizeof(search_info->pv_table_length));
        search_info->depth = iterative_depth;

        if (iterative_depth >= MIN_ASP_DEPTH) {
            delta = INITIAL_ASP_WINDOW;
            alpha = MAX(score - delta, -INF);
            beta = MIN(score + delta, INF);
        }

        int new_score = 0;

        while (!search_info->stopped) {
            new_score = search(alpha, beta, iterative_depth, pos, search_info, false);

            if (new_score <= alpha) {
                alpha = MAX(new_score - delta, -INF);
            }
            else if (new_score >= beta) {
                beta = MIN(new_score + delta, INF);
            }
            else {
                break;
            }
            delta *= 2;
        }

        // If time is up, and we have completed at least depth 1 search, break out of loop
        if (!search_info->pv_table_length[0] || (search_info->stopped && iterative_depth > 1))
            break;

        score = new_score;
        best_move = search_info->pv_table[0][0];

        unsigned int finish = get_time_ms();
        search_info->ms = (finish - start) > 0 ? finish - start : 1;
        report_search_info(search_info, score);
    }
    // Print the best move
    printf("bestmove ");
    print_move(best_move);
    printf("\n");
}
