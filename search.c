#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "movegen.h"
#include "move.h"
#include "search.h"

int NUM_THREADS = 1;

void scoreMoves(MoveList *moves, GameState *pos, int depth, SearchInfo *info)
{
	int i;
	int ply = info->depth - depth;
	for (i = 0; i < moves->nextOpen; i++)
	{
		// Score PV move
		if (moveEquality(moves->list[i], info->pvTable[0][ply]))
		{
			moves->list[i].score = 100000;
		}
		
		// Score captures
		else if (moves->list[i].prop & IS_CAPTURE)
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
			if (moveEquality(moves->list[i], info->killerMoves[0][ply]))
			{
				moves->list[i].score = 900;
			}
			// Check Second Killer Move
			else if (moveEquality(moves->list[i], info->killerMoves[1][ply]))
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
	int ply = info->depth - depth;
	info->pvTableLength[ply] = depth;
	
	if (depth <= 0 || ply >= MAX_PLY)
	{
		return quiescence(alpha, beta, info->depth, pos, info);
	}
	
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
				if (!(current.prop & IS_CAPTURE))
				{
					#pragma omp critical
					{
						if (!moveEquality(current, info->killerMoves[0][ply]))
						{
							info->killerMoves[1][ply] = info->killerMoves[0][ply];
							info->killerMoves[0][ply] = current;
						}
						info->history[newState.turn][current.src][current.dst] += (ply * ply);
					}
					
				}
				return beta;
			}				
			if (eval > alpha)
			{
				info->pvTable[ply][ply] = current;
				if (depth > 1)
				{
					// Crazy memcpy which copies PV from lower depth to current depth
					memcpy((info->pvTable[ply]) + ply + 1, (info->pvTable[ply + 1]) + ply + 1, info->pvTableLength[ply + 1] * sizeof(Move));
					info->pvTableLength[ply] = info->pvTableLength[ply + 1] + 1;
				}
				alpha = eval;
			}
		}
	}
	
	if (found == 0)
	{
		// When no more legal moves, the pv does not exist
		info->pvTableLength[ply] = 0;
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
	
	return alpha;
}

void search(int depth, GameState *pos, SearchInfo *rootInfo)
{
	int bestScore;
	double start, finish;
	
	
	// Clear information for rootInfo. Will have to do this for ID upon each depth
	rootInfo->nodes = 0ULL;
	memset(rootInfo->killerMoves, 0, sizeof(rootInfo->killerMoves));
	memset(rootInfo->history, 0, sizeof(rootInfo->history));
	memset(rootInfo->pvTable, 0, sizeof(rootInfo->pvTable));
	memset(rootInfo->pvTableLength, 0, sizeof(rootInfo->pvTableLength));
	
	for (int ID = 1; ID <= depth; ID++)
	{
		start = omp_get_wtime();
		rootInfo->nodes = 0ULL;
		rootInfo->depth = ID;
		bestScore = negaMax(-CHECKMATE, CHECKMATE, ID, pos, rootInfo);
		
		// After searching all possible moves, compile stats
		finish = omp_get_wtime() + 0.0001;
		rootInfo->ms = (unsigned int)((finish - start) * 1000);
		rootInfo->nps = (unsigned int)(rootInfo->nodes / (finish - start));
		rootInfo->bestScore = bestScore;
		
		int mated = 0;
		if (rootInfo->bestScore > MAX_PLY_CHECKMATE)
		{
			rootInfo->bestScore = CHECKMATE - rootInfo->bestScore;
			mated = 1;
		}
		else if (rootInfo->bestScore < -MAX_PLY_CHECKMATE)
		{
			rootInfo->bestScore = -CHECKMATE - rootInfo->bestScore;
			mated = 1;
		}
		
		printf("info depth %d ", ID);
		printf("score %s %d ", (mated) ? "mate" : "cp", rootInfo->bestScore);
		printf("time %u ", rootInfo->ms);
		printf("nodes %llu ", rootInfo->nodes);
		printf("nps %u ", rootInfo->nps);
		printf("pv");
		for (int i = 0; i < rootInfo->pvTableLength[0]; i++)
		{
			printf(" ");
			printMove(&(rootInfo->pvTable[0][i]));
		}
		printf("\n");
	
	}
	// Print the best move
	printf("bestmove ");
	printMove(&(rootInfo->pvTable[0][0]));
	printf("\n");
}
