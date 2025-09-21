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

int see(GameState *pos, int square)
{
	int value = 0;
	int piece = get_smallest_attacker(pos, square);
	if (piece != -1) {
		// Make capture
		// value = MAX(0, piece_captured - see(square, newPos));
		// Undo capture
	}
	return value;
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
		int matPawns = countBits(pos->pieceBitboards[P]) + countBits(pos->pieceBitboards[p]);
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
