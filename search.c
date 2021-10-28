#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "movegen.h"
#include "move.h"
#include "search.h"

int NUM_THREADS = 12;

int quiescence(int alpha, int beta, int depth, GameState *pos, SearchInfo *info)
{
	MoveList moveList;
	int size, i, legal, found = 0;
	int moveScores[256];
	
	int score = evaluation(pos);
	
	//printf("turn: %d, score: %d, alpha: %d, beta: %d eyyyy\n", pos->turn, score, alpha, beta);
	if (score >= beta)
	{
		return beta;
	}				
	if (score > alpha)
	{
		alpha = score;
	}
	
	memset(moveScores, 0, 256 * sizeof(int));
	moveList = generateMoves(pos, &size);
	qsort(moveList.list, size, sizeof(Move), compareMoves);
	
	for (i = 0; i < size; i++)
	{
		Move current = moveList.list[i];
		if ((current.prop & IS_CAPTURE) == 0)
		{
			continue;
		}
		GameState newState = playMove(pos, current, &legal);
		if (legal == 1)
		{
			found = 1;
			#pragma omp atomic
			info->nodes++;
			moveScores[i] = -quiescence(-beta, -alpha, depth + 1, &newState, info);
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
	
	// Draw by 50 move rule
	if (pos->halfMoveClock == 100)
	{
		return 0;
	}
	
	return alpha;
}

int negaMax(int alpha, int beta, int depth, GameState *pos, SearchInfo *info)
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
			#pragma omp atomic
			info->nodes++;
			moveScores[i] = -negaMax(-beta, -alpha, depth - 1, &newState, info);
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
			int mateDepth = (info->depth - depth + 1) / 2;
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
		return quiescence(alpha, beta, info->depth, pos, info);
	}
	
	return alpha;
}

Move search(int depth, GameState *pos, SearchInfo *info)
{
	MoveList moveList;
	int size, i;
	int bestScore, bestIndex;
	int moveScores[256];
	double start, finish;
	start = omp_get_wtime();
	info->nodes = 0ULL;
	
	memset(moveScores, 0, 256 * sizeof(int));
	moveList = generateMoves(pos, &size);
	qsort(moveList.list, size, sizeof(Move), compareMoves);
	
	bestIndex = 0;
	bestScore = -CHECKMATE;
	#pragma omp parallel for num_threads(NUM_THREADS) shared(bestIndex, bestScore, moveList)
	for (i = 0; i < size; i++)
	{
		int legal;
		Move current = moveList.list[i];
		GameState newState = playMove(pos, current, &legal);
		if (legal == 1)
		{
			#pragma omp atomic
			info->nodes++;
			moveScores[i] = -negaMax(-CHECKMATE, CHECKMATE, depth - 1, &newState, info);
			if (moveScores[i] > bestScore)
			{
				#pragma omp critical
				bestIndex = i;
				#pragma omp critical
				bestScore = moveScores[i];
			}
		}
	}
	finish = omp_get_wtime() + 0.0001;
	info->ms = (unsigned int)((finish - start) * 1000);
	info->nps = (unsigned int)(info->nodes / (finish - start));
	info->bestScore = bestScore;
	
	return moveList.list[bestIndex];
}
