#include <stdio.h>
#include <stdlib.h>
#include "position.h"
#include "evaluation.h"
#include "wrapper.h"

int materialCount(GameState *pos)
{
	int i;
	int score = 0;
	U64 pieceBB;
	for (i = P; i <= k; i++)
	{
		pieceBB = pos->pieceBitboards[i];
		while (pieceBB)
		{
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

int nnue_eval(GameState *pos)
{
	int i, idx = 2;
	U64 pieceBB;
	int pieces[65];
	int squares[65];
	for (i = P; i <= k; i++)
	{
		pieceBB = pos->pieceBitboards[i];
		while (pieceBB)
		{
			int sqr = getFirstBitSquare(pieceBB);
			if (i == K)
			{
				pieces[0] = nnue_pieces[i];
				squares[0] = sqr;
			}
			else if (i == k)
			{
				pieces[1] = nnue_pieces[i];
				squares[1] = sqr;
			}
			else
			{
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
	int score = 0;
	if (FOUND_NETWORK)
		score = nnue_eval(pos);
	else
		score += materialCount(pos);
	return score;
}

void printEvaluation(int score)
{
	int mated = 0;
	if (score > MAX_PLY_CHECKMATE)
	{
		score = CHECKMATE - score;
		mated = 1;
	}
	else if (score < -MAX_PLY_CHECKMATE)
	{
		score = -CHECKMATE - score;
		mated = 1;
	}
	printf("Eval: %s%d\n", (mated) ? "#" : "", score);
}