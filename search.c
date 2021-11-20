#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "movegen.h"
#include "move.h"
#include "search.h"

int NUM_THREADS = 1;
int followingPV = 0;

void scoreMoves(MoveList *moves, GameState *pos, int depth, SearchInfo *info)
{
	int i, scorePV = 0;
	int ply = info->depth - depth;
	
	// Only give PV node a score if actually following PV line
	if (followingPV)
	{
		followingPV = 0;
		for (int i = 0; i < moves->nextOpen; i++)
		{
			if (moves->list[i] == info->pvTable[0][ply])
			{
				followingPV = 1;
				scorePV = 1;
			}
		}
	}
	
	for (i = 0; i < moves->nextOpen; i++)
	{
		// Score PV move
		if (followingPV && scorePV && moves->list[i] == info->pvTable[0][ply])
		{
			moves->score[i] = 100000;
			scorePV = 0;
			continue;
		}
		
		// Score captures
		if (moves->list[i] & IS_CAPTURE)
		{
			int offset = 6 * pos->turn;
			moves->score[i] = MVV_LVA_TABLE[GET_MOVE_PIECE(moves->list[i]) - offset][GET_MOVE_CAPTURED(moves->list[i]) - 1] + 1000;
		}
		// Score quiet moves
		else
		{	
			// Check First Killer Move
			if (moves->list[i] == info->killerMoves[0][ply])
			{
				moves->score[i] = 900;
			}
			// Check Second Killer Move
			else if (moves->list[i] == info->killerMoves[1][ply])
			{
				moves->score[i] = 800;
			}
			else
			{
				moves->score[i] = info->history[pos->turn][GET_MOVE_SRC(moves->list[i])][GET_MOVE_DST(moves->list[i])];
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
		if (moves->score[i] > moves->score[bestIndex])
			bestIndex = i;
	}
	
	Move temp = moves->list[startIndex];
	int tempScore = moves->score[startIndex];
	moves->list[startIndex] = moves->list[bestIndex];
	moves->score[startIndex] = moves->score[bestIndex];
	moves->list[bestIndex] = temp;
	moves->score[bestIndex] = tempScore;
}

inline int okToReduce(Move move, GameState *parent, GameState *child)
{
	return (GET_MOVE_CAPTURED(move) == 0 && GET_MOVE_PROMOTION(move) == 0 && isInCheck(parent) == 0 && isInCheck(child) == 0);
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
		if ((GET_MOVE_CAPTURED(current) | GET_MOVE_PROMOTION(current)) == 0)
		{
			continue;
		}
		GameState newState = playMove(pos, current, &legal);
		if (legal == 1)
		{
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

int negaMax(int alpha, int beta, int depth, int nullMove, GameState *pos, SearchInfo *info)
{
	MoveList moveList;
	int size, i, legal, found = 0;
	int eval;
	int ply = info->depth - depth;
	int enablePVSearch = 0;
	int movesSearched = 0;
	info->pvTableLength[ply] = depth;
	
	if (depth <= 0 || ply >= MAX_PLY)
	{
		return quiescence(alpha, beta, info->depth, pos, info);
	}
	
	// Null move pruning. Something isn't working right
	/*if (nullMove && isInCheck(pos) == 0 && depth >= 3)
	{
		GameState newPos;
		memcpy(&newPos, pos, sizeof(GameState));
		// Make null move by switching side
		newPos.turn ^= 1;

		eval = -negaMax(-beta, -beta + 1, depth - 3, 0, &newPos, info);

	    if (eval >= beta)
	    {
	        return beta;
	    }
	}*/

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
			
			info->nodes++;
			
			// LMR psuedocode comes from https://web.archive.org/web/20150212051846/http://www.glaurungchess.com/lmr.html
			// via Tord Romstad
			if (movesSearched != 0)
			{
				if (movesSearched >= FULL_DEPTH_MOVES && depth >= REDUCTION_LIMIT && okToReduce(current, pos, &newState))
					
				{
					eval = -negaMax(-alpha - 1, -alpha, depth - 2, 1, &newState, info);
				}
				else
				{
					eval = alpha + 1;
				}
				
				if (eval > alpha)
				{
					eval = -negaMax(-alpha - 1, -alpha, depth - 1, 1, &newState, info);
					if ((eval > alpha) && (eval < beta))
					{
						eval = -negaMax(-beta, -alpha, depth - 1, 1, &newState, info);
					}
				}
			}
			else if (movesSearched == 0)
			{
				eval = -negaMax(-beta, -alpha, depth - 1, 1, &newState, info);
			}
			
			movesSearched++;
			if (eval >= beta)
			{
				if ((current & IS_CAPTURE) == 0)
				{
					if (current != info->killerMoves[0][ply])
					{
						info->killerMoves[1][ply] = info->killerMoves[0][ply];
						info->killerMoves[0][ply] = current;
					}
					info->history[newState.turn][GET_MOVE_SRC(current)][GET_MOVE_DST(current)] += (ply * ply);
				}
				return beta;
			}				
			if (eval > alpha)
			{
				enablePVSearch = 1;
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
		followingPV = 1;
		bestScore = negaMax(-CHECKMATE, CHECKMATE, ID, 1, pos, rootInfo);
		
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
			printMove((rootInfo->pvTable[0][i]));
		}
		printf("\n");
	
	}
	// Print the best move
	printf("bestmove ");
	printMove((rootInfo->pvTable[0][0]));
	printf("\n");
}
