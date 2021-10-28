#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "movegen.h"
#include "move.h"
#include "search.h"

int NUM_THREADS = 12;

int negaMax(int alpha, int beta, int startDepth, int depth, GameState *pos)
{
	MoveList moveList;
	int size, i, legal, found = 0;
	int moveScores[256];
	
	memset(moveScores, 0, 256 * sizeof(int));
	moveList = generateMoves(pos, &size);
	qsort(moveList.list, size, sizeof(Move), compareMoves);
	
	for (i = 0; i < size; i++)
	{
		Move current = moveList.list[i];
		GameState newState = playMove(pos, current, &legal);
		if (legal == 1)
		{
			found = 1;
			if (depth == 0)
				break;
			moveScores[i] = -negaMax(-beta, -alpha, startDepth, depth - 1, &newState);
			if (moveScores[i] >= beta)
			{
				return beta;
			}				
			if (moveScores[i] > alpha)
			{
				alpha = moveScores[i];
			}
		}
	}
	
	if (found == 0)
	{
		int offset = 6 * pos->turn;
		int kingLocation = getFirstBitSquare(pos->pieceBitboards[K + offset]);
		if (isSquareAttacked(pos, kingLocation, (pos->turn == WHITE) ? BLACK : WHITE))
		{
			int mateDepth = (startDepth - depth + 1) / 2;
			return -CHECKMATE + mateDepth;
		}
		return 0;
	}
	
	// Draw by 50 move rule
	if (pos->halfMoveClock == 100)
	{
		return 0;
	}
	
	if (depth == 0)
	{
		return evaluation(pos);
	}
	
	return alpha;
}

Move search(int depth, GameState *pos, int *score)
{
	MoveList moveList;
	int size, i, legal;
	int bestScore, bestIndex;
	int moveScores[256];
	
	memset(moveScores, 0, 256 * sizeof(int));
	moveList = generateMoves(pos, &size);
	qsort(moveList.list, size, sizeof(Move), compareMoves);
	
	bestIndex = 0;
	bestScore = -CHECKMATE;
	#pragma omp parallel for num_threads(NUM_THREADS) shared(bestIndex, bestScore, moveList) private(legal)
	for (i = 0; i < size; i++)
	{
		Move current = moveList.list[i];
		GameState newState = playMove(pos, current, &legal);
		if (legal == 1)
		{
			moveScores[i] = -negaMax(-CHECKMATE, CHECKMATE, depth, depth - 1, &newState);
			if (moveScores[i] > bestScore)
			{
				#pragma omp critical
				bestIndex = i;
				#pragma omp critical
				bestScore = moveScores[i];
			}
		}
	}
	*score = bestScore;
	return moveList.list[bestIndex];
}
