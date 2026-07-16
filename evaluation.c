#include "evaluation.h"
#include <stdio.h>
#include <stdlib.h>

#include "position.h"
#include "wrapper.h"

int INF = 30000;
int NO_SCORE = -30000;
int MATE_SCORE = 29900;
int MAX_MATE_SCORE = 29600;

int material_count(const GameState *pos)
{
    int eval = 0;
    for (int i = P; i <= k; i++) {
        u64 piece_bb = pos->piece_bitboards[i];
        while (piece_bb) {
            int sqr = GET_FIRST_BIT_SQUARE(piece_bb);
            eval += piece_value[i];
            if (i == P)
                eval += pawn_score[mirrored_square[sqr]];
            else if (i == p)
                eval -= pawn_score[sqr];
            else if (i == N)
                eval += knight_score[mirrored_square[sqr]];
            else if (i == n)
                eval -= knight_score[sqr];
            else if (i == B)
                eval += bishop_score[mirrored_square[sqr]];
            else if (i == b)
                eval -= bishop_score[sqr];
            else if (i == R)
                eval += rook_score[mirrored_square[sqr]];
            else if (i == r)
                eval -= rook_score[sqr];
            else if (i == K)
                eval += king_score[mirrored_square[sqr]];
            else if (i == k)
                eval -= king_score[sqr];
            CLEAR_LSB(piece_bb);
        }
    }
    return (pos->turn == WHITE) ? eval : -eval;
}

int material(const GameState *pos)
{
    int m = 0;
    m += COUNT_BITS(pos->piece_bitboards[P]) * piece_value[P];
    m += COUNT_BITS(pos->piece_bitboards[N]) * piece_value[N];
    m += COUNT_BITS(pos->piece_bitboards[B]) * piece_value[B];
    m += COUNT_BITS(pos->piece_bitboards[R]) * piece_value[R];
    m += COUNT_BITS(pos->piece_bitboards[Q]) * piece_value[Q];

    m += COUNT_BITS(pos->piece_bitboards[p]) * piece_value[p];
    m += COUNT_BITS(pos->piece_bitboards[n]) * piece_value[n];
    m += COUNT_BITS(pos->piece_bitboards[b]) * piece_value[b];
    m += COUNT_BITS(pos->piece_bitboards[r]) * piece_value[r];
    m += COUNT_BITS(pos->piece_bitboards[q]) * piece_value[q];
    return m;
}

static void build_nnue_arrays(const GameState *pos, int *pieces, int *squares)
{
    int idx = 2;
    for (int i = P; i <= k; i++) {
        u64 piece_bb = pos->piece_bitboards[i];
        while (piece_bb) {
            int sqr = GET_FIRST_BIT_SQUARE(piece_bb);
            if (i == K) {
                pieces[0] = nnue_pieces[i];
                squares[0] = sqr;
            }
            else if (i == k) {
                pieces[1] = nnue_pieces[i];
                squares[1] = sqr;
            }
            else {
                pieces[idx] = nnue_pieces[i];
                squares[idx] = sqr;
                idx++;
            }
            CLEAR_LSB(piece_bb);
        }
    }
    pieces[idx] = 0;
    squares[idx] = 0;
}

int nnue_eval(const GameState *pos)
{
    int pieces[65];
    int squares[65];
    build_nnue_arrays(pos, pieces, squares);
    return evaluate_nnue(pos->turn, pieces, squares);
}

// Rebuilds an accumulator from scratch directly off the board's bitboards, bypassing the
// pieces[]/squares[] array entirely (used on a king move or the very first eval of a search).
void nnue_refresh_from_gamestate(Accumulator *acc, const GameState *pos)
{
    int white_ksq = GET_FIRST_BIT_SQUARE(pos->piece_bitboards[K]);
    int black_ksq = GET_FIRST_BIT_SQUARE(pos->piece_bitboards[k]);
    int pieces[32], squares[32];
    int count = 0;
    for (int i = P; i <= k; i++) {
        if (i == K || i == k)
            continue; // kings aren't features, only used for orientation
        u64 piece_bb = pos->piece_bitboards[i];
        while (piece_bb) {
            pieces[count] = nnue_pieces[i];
            squares[count] = GET_FIRST_BIT_SQUARE(piece_bb);
            count++;
            CLEAR_LSB(piece_bb);
        }
    }
    nnue_refresh_features(acc, pieces, squares, count, white_ksq, black_ksq);
    acc->computedAccumulation = 1;
}

// Applies a DirtyPiece's feature changes on top of a valid parent accumulator, in one
// combined pass fused with the copy-from-parent step (rather than copying first and then
// modifying, or one call per changed feature). Caller must have already ruled out a king
// move (that needs a refresh).
void apply_dirty_piece(Accumulator *acc, const Accumulator *parent, const DirtyPiece *dp,
                       int white_ksq, int black_ksq)
{
    int removed_pieces[3], removed_squares[3], n_removed = 0;
    int added_pieces[3], added_squares[3], n_added = 0;
    for (int i = 0; i < dp->dirtyNum; i++) {
        if (IS_KING(dp->pc[i]))
            continue;
        if (dp->from[i] != 64) {
            removed_pieces[n_removed] = dp->pc[i];
            removed_squares[n_removed] = dp->from[i];
            n_removed++;
        }
        if (dp->to[i] != 64) {
            added_pieces[n_added] = dp->pc[i];
            added_squares[n_added] = dp->to[i];
            n_added++;
        }
    }
    nnue_apply_features(acc, parent, removed_pieces, removed_squares, n_removed,
                         added_pieces, added_squares, n_added, white_ksq, black_ksq);
    acc->computedAccumulation = 1;
}

// Fills a DirtyPiece describing exactly which NNUE features change for a given move,
// mirroring make_move's own move-category logic so both stay in lockstep.
void compute_dirty_piece(const GameState *pos, Move move, DirtyPiece *dp)
{
    int turn = pos->turn;
    int offset = 6 * turn;
    int piece = GET_MOVE_PIECE(move);
    int src = GET_MOVE_SRC(move);
    int dst = GET_MOVE_DST(move);
    int promotion = GET_MOVE_PROMOTION(move);
    int victim = GET_MOVE_CAPTURED(move);

    dp->dirtyNum = 0;

#define ADD_DIRTY(engine_piece, from_sq, to_sq) \
    do { \
        dp->pc[dp->dirtyNum] = nnue_pieces[(engine_piece)]; \
        dp->from[dp->dirtyNum] = (from_sq); \
        dp->to[dp->dirtyNum] = (to_sq); \
        dp->dirtyNum++; \
    } while (0)

    if ((piece == P || piece == p) && IS_MOVE_EP(move)) {
        int ep_square = (turn == WHITE) ? dst - 8 : dst + 8;
        int ep_pawn = p - offset;
        ADD_DIRTY(piece, src, dst);
        ADD_DIRTY(ep_pawn, ep_square, 64);
    }
    else if (piece == (K + offset) && IS_MOVE_CASTLES(move)) {
        ADD_DIRTY(piece, src, dst);
        int rook = R + offset;
        if (src < dst)
            ADD_DIRTY(rook, dst + 1, dst - 1);
        else
            ADD_DIRTY(rook, dst - 2, dst + 1);
    }
    else {
        if (promotion) {
            ADD_DIRTY(piece, src, 64);
            ADD_DIRTY(promotion + offset, 64, dst);
        }
        else {
            ADD_DIRTY(piece, src, dst);
        }
        if (victim != NO_CAPTURE) {
            ADD_DIRTY(victim, dst, 64);
        }
    }

#undef ADD_DIRTY
}

int evaluation(const GameState *pos)
{
    if (FOUND_NETWORK) {
        int mat = material(pos);
        return nnue_eval(pos) * (720 + mat / 32) / 1024 + 28;
    }
    return material_count(pos);
}

int evaluate_from_accumulator(const GameState *pos, const Accumulator *acc)
{
    if (FOUND_NETWORK) {
        int mat = material(pos);
        return nnue_evaluate_accumulator(acc, pos->turn) * (720 + mat / 32) / 1024 + 28;
    }
    return material_count(pos);
}

// Updates the accumulator at nnue_stack[ply] (copying from its parent and applying the
// move's DirtyPiece deltas directly, or refreshing from scratch on a king move / missing
// parent) and evaluates it, all without ever building a pieces[]/squares[] array.
int evaluate_at_ply(NNUEdata *nnue_stack, int ply, const GameState *pos)
{
    Accumulator *acc = &nnue_stack[ply].accumulator;

    if (!acc->computedAccumulation) {
        DirtyPiece *dp = &nnue_stack[ply].dirtyPiece;
        bool king_moved = dp->dirtyNum > 0 && IS_KING(dp->pc[0]);
        Accumulator *parent = (!king_moved && ply >= 1) ? &nnue_stack[ply - 1].accumulator : NULL;

        if (parent && parent->computedAccumulation) {
            int white_ksq = GET_FIRST_BIT_SQUARE(pos->piece_bitboards[K]);
            int black_ksq = GET_FIRST_BIT_SQUARE(pos->piece_bitboards[k]);
            apply_dirty_piece(acc, parent, dp, white_ksq, black_ksq);
        }
        else {
            nnue_refresh_from_gamestate(acc, pos);
        }
    }

    return evaluate_from_accumulator(pos, acc);
}

void print_evaluation(int score)
{
    int mated = 0;
    if (score > MATE_SCORE) {
        score = INF - score;
        mated = 1;
    }
    else if (score < -MATE_SCORE) {
        score = -INF - score;
        mated = 1;
    }
    printf("Eval: %s%d\n", (mated) ? "#" : "", score);
}
