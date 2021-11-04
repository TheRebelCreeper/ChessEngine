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
	int size, i, legal;
	
	int eval = evaluation(pos);
	
	if (eval >= beta)
	{
		return beta;
	}				
	if (eval > alpha)
	{
		alpha = eval;
	}
	
	// Stop at qsearch
	if (depth >= (info->depth * 2 + 1) && !(isInCheck(pos)))
		return alpha;
	
	moveList = generateMoves(pos, &size);
	
	for (i = 0; i < size; i++)
	{
		Move current = moveList.list[i];
		if ((current.prop & (IS_CAPTURE | IS_PROMOTION)) == 0)
		{
			continue;
		}
		GameState newState = playMove(pos, current, &legal);
		if (legal == 1)
		{
			#pragma omp atomic
			info->nodes++;
			eval = -quiescence(-beta, -alpha, depth + 1, &newState, info);
			if (eval >= beta)
			{
				return beta;
			}				
			if (eval > alpha)
			{
				alpha = eval;
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
	int eval;
	
	moveList = generateMoves(pos, &size);
	scoreMoves(&moveList, pos);
	qsort(moveList.list, size, sizeof(Move), compareMoves);
	
	for (i = 0; i < size; i++)
	{
		// TODO get current using list.pickMove, this will avoid requiring qsort
		Move current = moveList.list[i];
		GameState newState = playMove(pos, current, &legal);
		if (legal == 1)
		{
			found = 1;
			if (depth <= 0)
				break;
			#pragma omp atomic
			info->nodes++;
			eval = -negaMax(-beta, -alpha, depth - 1, &newState, info);
			if (eval >= beta)
			{
				return beta;
			}				
			if (eval > alpha)
			{
				alpha = eval;
			}
		}
	}
	
	if (found == 0)
	{
		if (isInCheck(pos))
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
	
	if (depth <= 0)
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
	int eval;
	double start, finish;
	start = omp_get_wtime();
	info->nodes = 0ULL;
	
	moveList = generateMoves(pos, &size);
	scoreMoves(&moveList, pos);
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
			eval = -negaMax(-CHECKMATE, CHECKMATE, depth - 1, &newState, info);
			if (eval > bestScore)
			{
				#pragma omp critical
				bestIndex = i;
				#pragma omp critical
				bestScore = eval;
			}
		}
	}
	finish = omp_get_wtime() + 0.0001;
	info->ms = (unsigned int)((finish - start) * 1000);
	info->nps = (unsigned int)(info->nodes / (finish - start));
	info->bestScore = bestScore;
	
	return moveList.list[bestIndex];
}
