#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "movegen.h"
#include "move.h"
#include "search.h"

int NUM_THREADS = 4;

void scoreMoves(MoveList *moves, GameState *pos, int depth, SearchInfo *info)
{
	int i;
	for (i = 0; i < moves->nextOpen; i++)
	{
		// Score captures
		if (moves->list[i].prop & IS_CAPTURE)
		{
			int offset = 6 * (pos->turn ^ 1);
			int victim = P;
			for (int j = P; j <= K; j++)
			{
				if (get_square(pos->pieceBitboards[j + offset], moves->list[i].dst))
				{
					victim = j;
					break;
				}
			}
			offset = 6 * pos->turn;
			moves->list[i].score = MVV_LVA_TABLE[moves->list[i].piece - offset][victim] + 1000;
		}
		// Score quiet moves
		else
		{	
			// Check First Killer Move
			if (moveEquality(moves->list[i], info->killerMoves[0][depth]))
			{
				moves->list[i].score = 900;
			}
			// Check Second Killer Move
			else if (moveEquality(moves->list[i], info->killerMoves[1][depth]))
			{
				moves->list[i].score = 800;
			}
			else
			{
				moves->list[i].score = info->history[pos->turn][moves->list[i].src][moves->list[i].dst];
			}
		}
	}
}

void pickMove(MoveList *moves, int startIndex)
{
	int bestIndex = startIndex;
	int i;
	for (i = startIndex; i < moves->nextOpen; i++)
	{
		if (moves->list[i].score > moves->list[bestIndex].score)
			bestIndex = i;
	}
	
	Move temp = moves->list[startIndex];
	moves->list[startIndex] = moves->list[bestIndex];
	moves->list[bestIndex] = temp;
}

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
	
	moveList = generateMoves(pos, &size);
	scoreMoves(&moveList, pos, depth, info);
	
	for (i = 0; i < size; i++)
	{
		pickMove(&moveList, i);
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
	scoreMoves(&moveList, pos, depth, info);
	
	for (i = 0; i < size; i++)
	{
		pickMove(&moveList, i);
		Move current = moveList.list[i];
		GameState newState = playMove(pos, current, &legal);
		if (legal == 1)
		{
			found = 1;
			if (depth <= 0)
				break;
			
			// TODO check extentions result in displaying wrong mate eval
			// TODO check extentions can cause infinite loop when threefold repetition possible
			// k7/pp6/8/2r5/8/8/1K2R3/8 w - - 0 1
			//if (isInCheck(pos))
			//	depth++;
			
			#pragma omp atomic
			info->nodes++;
			eval = -negaMax(-beta, -alpha, depth - 1, &newState, info);
			if (eval >= beta)
			{
				// There is a sychronization problem here because different threads are sharing killerMoves
				// It results in some branches being evaluated due to poor move ordering, but will not miss correct line
				if (!(current.prop & IS_CAPTURE))
				{
					#pragma omp critical
					{
						if (!moveEquality(current, info->killerMoves[0][depth]))
						{
							info->killerMoves[1][depth] = info->killerMoves[0][depth];
							info->killerMoves[0][depth] = current;
						}
						info->history[newState.turn][current.src][current.dst] += ((info->depth - depth) * (info->depth - depth));
					}
					
				}
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
	memset(info->killerMoves, 0, sizeof(info->killerMoves));
	memset(info->history, 0, sizeof(info->history));
	
	moveList = generateMoves(pos, &size);
	scoreMoves(&moveList, pos, depth, info);
	qsort(moveList.list, size, sizeof(Move), compareMoves);
	
	bestIndex = 0;
	bestScore = -CHECKMATE;
	#pragma omp parallel for num_threads(NUM_THREADS) shared(bestIndex, bestScore, moveList)
	for (i = 0; i < size; i++)
	{
		int legal;
		// Not sure how to sychronize this with OMP, probably barrier?
		//#pragma omp critical
		//pickMove(&moveList, i);
		
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
