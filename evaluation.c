#include "evaluation.h"
#include <stdio.h>
#include <stdlib.h>

#include "et.h"
#include "position.h"
#include "wrapper.h"

int materialCount(GameState *pos)
{
    int i;
    int score = 0;
    U64 pieceBB;
    for (i = P; i <= k; i++) {
        pieceBB = pos->pieceBitboards[i];
        while (pieceBB) {
            int sqr = getFirstBitSquare(pieceBB);
            score += pieceValue[i];
            if (i == P)
                score += pawnScore[mirroredSquare[sqr]];
            else if (i == p)
                score -= pawnScore[sqr];
            else if (i == N)
                score += knightScore[mirroredSquare[sqr]];
            else if (i == n)
                score -= knightScore[sqr];
            else if (i == B)
                score += bishopScore[mirroredSquare[sqr]];
            else if (i == b)
                score -= bishopScore[sqr];
            else if (i == R)
                score += rookScore[mirroredSquare[sqr]];
            else if (i == r)
                score -= rookScore[sqr];
            else if (i == K)
                score += kingScore[mirroredSquare[sqr]];
            else if (i == k)
                score -= kingScore[sqr];
            clear_lsb(pieceBB);
        }
    }
    return (pos->turn == WHITE) ? score : -score;
}

int nonPawnMaterial(GameState *pos)
{
    int mat = 0;
    mat += countBits(pos->pieceBitboards[N]) * pieceValue[N];
    mat += countBits(pos->pieceBitboards[B]) * pieceValue[B];
    mat += countBits(pos->pieceBitboards[R]) * pieceValue[R];
    mat += countBits(pos->pieceBitboards[Q]) * pieceValue[Q];

    mat += countBits(pos->pieceBitboards[n]) * pieceValue[n];
    mat += countBits(pos->pieceBitboards[b]) * pieceValue[b];
    mat += countBits(pos->pieceBitboards[r]) * pieceValue[r];
    mat += countBits(pos->pieceBitboards[q]) * pieceValue[q];
    return mat;
}

// Fast Static Exchange Evaluation (SEE)
int see(GameState *pos, int square)
{
    U64 occupied = pos->occupancies[BOTH]; // all pieces
    U64 attackers[2]; // white & black attackers
    int gain[32]; // swap values
    int depth = 0;

    // piece on the target square (initial capture value)
    int captured = pos->mailbox[square];
    if (captured == NO_PIECE)
        return 0;

    // Initialize attackers
    attackers[WHITE] = pawnAttacks[BLACK][square] & pos->pieceBitboards[P];
    attackers[WHITE] |= knightAttacks[square] & pos->pieceBitboards[N];
    attackers[WHITE] |= getBishopAttacks(square, occupied) & (pos->pieceBitboards[B] | pos->pieceBitboards[Q]);
    attackers[WHITE] |= getRookAttacks(square, occupied) & (pos->pieceBitboards[R] | pos->pieceBitboards[Q]);
    attackers[WHITE] |= kingAttacks[square] & pos->pieceBitboards[K];

    attackers[BLACK] = pawnAttacks[WHITE][square] & pos->pieceBitboards[p];
    attackers[BLACK] |= knightAttacks[square] & pos->pieceBitboards[n];
    attackers[BLACK] |= getBishopAttacks(square, occupied) & (pos->pieceBitboards[b] | pos->pieceBitboards[q]);
    attackers[BLACK] |= getRookAttacks(square, occupied) & (pos->pieceBitboards[r] | pos->pieceBitboards[q]);
    attackers[BLACK] |= kingAttacks[square] & pos->pieceBitboards[k];

    // swap list holds material balance after each exchange
    gain[depth] = abs(pieceValue[captured]);
    //printf("gain[%d] = %d\n", depth, gain[depth]);
    int stm = pos->turn; // side to move


    do {
        // find least valuable attacker for current side
        int attacker = NO_PIECE;
        int attackerSq = -1;
        int minValue = 99999;
        int offset = stm * 6;

        for (int pt = P; pt <= K; pt++) {
            U64 bb = pos->pieceBitboards[pt + offset] & attackers[stm];
            if (bb) {
                int sq = getFirstBitSquare(bb);
                int val = abs(pieceValue[pt + offset]);
                if (val < minValue) {
                    minValue = val;
                    attacker = pt + offset;
                    attackerSq = sq;
                }
            }
        }

        if (attacker == NO_PIECE)
            break;

        // next captured piece is the value of this attacker
        depth++;
        gain[depth] = -gain[depth - 1] + abs(pieceValue[attacker]);
        //printf("gain[%d] = %d\n", depth, gain[depth]);

        // remove attacker from board
        clear_square(occupied, attackerSq);

        // update attackers (sliders may now attack through)
        attackers[WHITE] = (attackers[WHITE] & occupied) |
                           (getBishopAttacks(square, occupied) & (pos->pieceBitboards[B] | pos->pieceBitboards[Q]) &
                            occupied) |
                           (getRookAttacks(square, occupied) & (pos->pieceBitboards[R] | pos->pieceBitboards[Q]) &
                            occupied);

        attackers[BLACK] = (attackers[BLACK] & occupied) |
                           (getBishopAttacks(square, occupied) & (pos->pieceBitboards[b] | pos->pieceBitboards[q]) &
                            occupied) |
                           (getRookAttacks(square, occupied) & (pos->pieceBitboards[r] | pos->pieceBitboards[q]) &
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

int nnue_eval(GameState *pos)
{
    int i, idx = 2;
    U64 pieceBB;
    int pieces[65];
    int squares[65];
    for (i = P; i <= k; i++) {
        pieceBB = pos->pieceBitboards[i];
        while (pieceBB) {
            int sqr = getFirstBitSquare(pieceBB);
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
            clear_lsb(pieceBB);
        }
    }
    pieces[idx] = 0;
    squares[idx] = 0;

    return evaluateNNUE(pos->turn, pieces, squares);
}

int evaluation(GameState *pos)
{
    int score = probeET(pos);

    if (score == INVALID_EVALUATION) {
        score = 0;
        int matPawns = countBits(pos->pieceBitboards[P] | pos->pieceBitboards[p]);
        int mat = nonPawnMaterial(pos) + matPawns * pieceValue[P];
        if (FOUND_NETWORK)
            score = nnue_eval(pos) * (720 + mat / 32) / 1024 + 28;
        else
            score += materialCount(pos);
        saveET(pos, score);
    }

    return score * (100 - pos->halfMoveClock) / 100;
}

void printEvaluation(int score)
{
    int mated = 0;
    if (score > CHECKMATE) {
        score = INF - score;
        mated = 1;
    }
    else if (score < -CHECKMATE) {
        score = -INF - score;
        mated = 1;
    }
    printf("Eval: %s%d\n", (mated) ? "#" : "", score);
}
