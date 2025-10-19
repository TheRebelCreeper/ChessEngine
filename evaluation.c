#include "evaluation.h"
#include <stdio.h>
#include <stdlib.h>

#include "et.h"
#include "position.h"
#include "wrapper.h"

int material_count(const GameState *pos)
{
    int eval = 0;
    for (int i = P; i <= k; i++) {
        U64 piece_bb = pos->piece_bitboards[i];
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

int non_pawn_material(const GameState *pos)
{
    int m = 0;
    m += COUNT_BITS(pos->piece_bitboards[N]) * piece_value[N];
    m += COUNT_BITS(pos->piece_bitboards[B]) * piece_value[B];
    m += COUNT_BITS(pos->piece_bitboards[R]) * piece_value[R];
    m += COUNT_BITS(pos->piece_bitboards[Q]) * piece_value[Q];

    m += COUNT_BITS(pos->piece_bitboards[n]) * piece_value[n];
    m += COUNT_BITS(pos->piece_bitboards[b]) * piece_value[b];
    m += COUNT_BITS(pos->piece_bitboards[r]) * piece_value[r];
    m += COUNT_BITS(pos->piece_bitboards[q]) * piece_value[q];
    return m;
}

// Fast Static Exchange Evaluation (SEE)
int see(const GameState *pos, int square)
{
    U64 occupied = pos->occupancies[BOTH]; // all pieces
    U64 attackers[2]; // white & black attackers
    int gain[32]; // swap values
    int depth = 0;

    // piece on the target square (initial capture value)
    int captured = pos->mailbox[square];
    if (captured == NO_PIECE) {
        return 0;
    }

    // Initialize attackers
    attackers[WHITE] = pawn_attacks[BLACK][square] & pos->piece_bitboards[P];
    attackers[WHITE] |= knight_attacks[square] & pos->piece_bitboards[N];
    attackers[WHITE] |= get_bishop_attacks(square, occupied) & (pos->piece_bitboards[B] | pos->piece_bitboards[Q]);
    attackers[WHITE] |= get_rook_attacks(square, occupied) & (pos->piece_bitboards[R] | pos->piece_bitboards[Q]);
    attackers[WHITE] |= king_attacks[square] & pos->piece_bitboards[K];

    attackers[BLACK] = pawn_attacks[WHITE][square] & pos->piece_bitboards[p];
    attackers[BLACK] |= knight_attacks[square] & pos->piece_bitboards[n];
    attackers[BLACK] |= get_bishop_attacks(square, occupied) & (pos->piece_bitboards[b] | pos->piece_bitboards[q]);
    attackers[BLACK] |= get_rook_attacks(square, occupied) & (pos->piece_bitboards[r] | pos->piece_bitboards[q]);
    attackers[BLACK] |= king_attacks[square] & pos->piece_bitboards[k];

    // swap list holds material balance after each exchange
    gain[depth] = abs(piece_value[captured]);
    //printf("gain[%d] = %d\n", depth, gain[depth]);
    int stm = pos->turn; // side to move

    do {
        // find least valuable attacker for current side
        int attacker = NO_PIECE;
        int attacker_sq = -1;
        int min_value = 99999;
        int offset = stm * 6;

        for (int i = P; i <= K; i++) {
            U64 bb = pos->piece_bitboards[i + offset] & attackers[stm];
            if (bb) {
                int sq = GET_FIRST_BIT_SQUARE(bb);
                int val = abs(piece_value[i + offset]);
                if (val < min_value) {
                    min_value = val;
                    attacker = i + offset;
                    attacker_sq = sq;
                }
            }
        }

        if (attacker == NO_PIECE) {
            break;
        }

        // next captured piece is the value of this attacker
        depth++;
        gain[depth] = -gain[depth - 1] + abs(piece_value[attacker]);
        //printf("gain[%d] = %d\n", depth, gain[depth]);

        // remove attacker from board
        CLEAR_SQUARE(occupied, attacker_sq);

        // update attackers (sliders may now attack through)
        attackers[WHITE] = (attackers[WHITE] & occupied) |
                           (get_bishop_attacks(square, occupied) & (pos->piece_bitboards[B] | pos->piece_bitboards[Q]) &
                            occupied) |
                           (get_rook_attacks(square, occupied) & (pos->piece_bitboards[R] | pos->piece_bitboards[Q]) &
                            occupied);

        attackers[BLACK] = (attackers[BLACK] & occupied) |
                           (get_bishop_attacks(square, occupied) & (pos->piece_bitboards[b] | pos->piece_bitboards[q]) &
                            occupied) |
                           (get_rook_attacks(square, occupied) & (pos->piece_bitboards[r] | pos->piece_bitboards[q]) &
                            occupied);

        stm = !stm; // switch side
    }
    while (depth < 31); // safety bound

    // minimax from the end
    while (--depth) {
        if (-gain[depth] < gain[depth - 1])
            gain[depth - 1] = -gain[depth];
    }

    return gain[0];
}

int nnue_eval(const GameState *pos)
{
    int idx = 2;
    int pieces[65];
    int squares[65];
    for (int i = P; i <= k; i++) {
        U64 piece_bb = pos->piece_bitboards[i];
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

    return evaluate_nnue(pos->turn, pieces, squares);
}

int evaluation(GameState *pos)
{
    int score = probe_et(pos);

    if (score == INVALID_EVALUATION) {
        score = 0;
        int mat_pawns = COUNT_BITS(pos->piece_bitboards[P] | pos->piece_bitboards[p]);
        int mat = non_pawn_material(pos) + mat_pawns * piece_value[P];
        if (FOUND_NETWORK)
            score = nnue_eval(pos) * (720 + mat / 32) / 1024 + 28;
        else
            score += material_count(pos);
        save_et(pos, score);
    }

    return score * (100 - pos->half_move_clock) / 100;
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
