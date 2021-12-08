#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <omp.h>

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#include <sys/select.h>
#endif

#include "movegen.h"
#include "move.h"
#include "search.h"
#include "tt.h"

int NUM_THREADS = 1;
int followingPV = 0;

void checkTimeLeft(SearchInfo *info) {
	// .. check if time up, or interrupt from GUI
	if(info->timeset == 1 && GetTimeMs() > info->stoptime)
	{
		info->stopped = 1;
	}
	
	ReadInput(info);
}

void scoreMoves(MoveList *moves, GameState *pos, Move ttMove, SearchInfo *info)
{
	int i, scorePV = 0;
	int ply = info->ply;
	
	for (i = 0; i < moves->nextOpen; i++)
	{
		
		// Score TT hits
		if (moves->list[i] == ttMove)
		{
			moves->score[i] = TT_HIT_SCORE;
			continue;
		}
		
		// Score captures
		if (moves->list[i] & IS_CAPTURE)
		{
			int offset = 6 * pos->turn;
			moves->score[i] = MVV_LVA_TABLE[GET_MOVE_PIECE(moves->list[i]) - offset][GET_MOVE_CAPTURED(moves->list[i]) - 1] + KILLER_ONE;
		}
		// Score quiet moves
		else
		{	
			// Check First Killer Move
			if (moves->list[i] == info->killerMoves[0][ply])
			{
				moves->score[i] = KILLER_ONE;
			}
			// Check Second Killer Move
			else if (moves->list[i] == info->killerMoves[1][ply])
			{
				moves->score[i] = KILLER_TWO;
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

inline int is_repetition(GameState *pos)
{
	int reps = 0;
	for (int i = 1; i < pos->halfMoveClock; i++)
	{
		int idx = historyIndex - i;
		if (idx < 0)
			break;
		if (posHistory[idx] == pos->key)
			reps++;
	}
	//return reps >= 2; //This handles true threefold instead of single rep
	return reps != 0; // Detects a single rep
}

inline int okToReduce(Move move, int inCheck, int givesCheck, int pv)
{
	return (GET_MOVE_CAPTURED(move) == 0 && GET_MOVE_PROMOTION(move) == 0 && inCheck == 0 && givesCheck == 0);
}

int negaMax(int alpha, int beta, int depth, GameState *pos, SearchInfo *info, int pruneNull)
{
	char nodeBound = TT_ALL;
	int size, i, legal, legalMoves = 0;
	int ply = info->ply;
	int inCheck = isInCheck(pos);
	int isPVNode = beta - alpha > 1;
	int isRoot = (ply == 0);
	int enableFutilityPruning = 0;
	
	MoveList moveList;
	Move bestMove = 0;
	
	info->pvTableLength[ply] = depth;
	info->nodes++;
	
	// Search has exceeded max depth, return static eval
	if (ply > MAX_PLY)
	{
		return evaluation(pos);
	}
	
	// Update time left
	if(( info->nodes & 2047 ) == 0)
	{
		checkTimeLeft(info);
		// Ran out of time
		if (info->stopped)
			return 0;
	}
	
	// Increase depth if currently in check since there are few replies
	#ifdef CHECK_EXTENTIONS
	if (inCheck)
		depth++;
	#endif
	
	// Enter quiescence if not in check
	if (depth <= 0)
	{
		info->nodes--;
		return quiescence(alpha, beta, depth, pos, info);
	}
	
	// Mate distance pruning
	// Stops from searching for mate when faster mate already found
	if (!isRoot)
	{
		int distance = -INF + ply;
		
		if (distance > alpha)
		{
			alpha = distance;
			if (beta <= distance)
				return distance;
		}
	}
	
	// Search for draws and repetitions
	// Don't need to search for repetition if halfMoveClock is low
	if (!isRoot && ((pos->halfMoveClock > 4 && is_repetition(pos)) || pos->halfMoveClock == 100))
	{
		// Cut the pv line if this is a draw
		info->pvTableLength[ply] = 0;
		return 0;
	}

	// Check hash table for best move
	// Do not cut if pv node
	Move ttMove = 0;
	int eval = probeTT(pos, &ttMove, alpha, beta, depth, ply);
	if (eval != INVALID_SCORE && !isRoot && !isPVNode)
	{
		return eval;
	}
	
	int staticEval = evaluation(pos);
	
	// Static Null Move Pruning
	// TODO Learn how this works
	if (depth < 3 && !isPVNode && !inCheck && abs(beta) < CHECKMATE)
	{   
		int evalMargin = 120 * depth;
		if (staticEval - evalMargin >= beta)
			return staticEval - evalMargin;
	}
	
	// Null move pruning
	if (pruneNull && !isPVNode && !inCheck && depth >= 3 && !onlyHasPawns(pos, pos->turn))
	{
		// Reduce by either 2 or 3 ply depending on depth
		int r = (depth <= 6) ? 2 : 3;
		
		// Make the null move
		GameState newPos;
		memcpy(&newPos, pos, sizeof(GameState));
		newPos.turn ^= 1;
		newPos.enpassantSquare = none;
		newPos.key ^= sideKey;
		if (pos->enpassantSquare != none)
			newPos.key ^= epKey[pos->enpassantSquare & 7];
		
		info->ply++;
		
		// Search resulting position with reduced depth
		eval = -negaMax(-beta, -beta + 1, depth - 1 - r, &newPos, info, 0);
		
		// Unmake null move
		info->ply--;
		
		// Ran out of time
		if (info->stopped)
			return 0;
		
		if (eval >= beta && abs(eval) < CHECKMATE)
			return beta;
	}

	// Could add futility pruning check here
	if (depth < 9 && !isPVNode && !inCheck && alpha < CHECKMATE)
	{
		if (staticEval + futilityMargins[depth] <= alpha)
			enableFutilityPruning = 1;
	}

	moveList = generateMoves(pos, &size);
	scoreMoves(&moveList, pos, ttMove, info);
	
	for (i = 0; i < size; i++)
	{
		pickMove(&moveList, i);
		Move current = moveList.list[i];
		GameState newState = playMove(pos, current, &legal);

		// Increment history index to next depth
		
		if (!legal)
			continue;
		
		legalMoves++;
		info->ply++;
		
		// Save the current move into history
		historyIndex++;
		posHistory[historyIndex] = newState.key;
		
		// Does this move give check
		int givesCheck = isInCheck(&newState);
		
		// Futility Pruning
		if (enableFutilityPruning && legalMoves > 1)
		{
			if (!givesCheck && !GET_MOVE_PROMOTION(current) && !GET_MOVE_CAPTURED(current))
			{
				info->ply--;
				historyIndex--;
				continue;
			}
		}
		
		// LMR psuedocode comes from https://web.archive.org/web/20150212051846/http://www.glaurungchess.com/lmr.html
		// via Tord Romstad
		
		// Do a full depth search on PV
		if (legalMoves == 1)
		{
			eval = -negaMax(-beta, -alpha, depth - 1, &newState, info, 1);
		}
		// LMR on non PV node
		else
		{
			if (legalMoves >= FULL_DEPTH_MOVES && depth >= REDUCTION_LIMIT && okToReduce(current, inCheck, givesCheck, isPVNode))
			{
				// Reduced search without null moves
				eval = -negaMax(-alpha - 1, -alpha, depth - 2, &newState, info, 1);
			}
			else
			{
				eval = alpha + 1;
			}
			
			if (eval > alpha)
			{
				eval = -negaMax(-alpha - 1, -alpha, depth - 1, &newState, info, 1);
				if ((eval > alpha) && (eval < beta))
				{
					eval = -negaMax(-beta, -alpha, depth - 1, &newState, info, 1);
				}
			}
		}
		
		// Unmake move by removing current move from history
		info->ply--;
		historyIndex--;

		if (info->stopped)
			return 0;
		
		if (eval > alpha)
		{		
			nodeBound = TT_PV;
			info->pvTable[ply][ply] = current;
			if (depth > 1)
			{
				// Crazy memcpy which copies PV from lower depth to current depth
				memcpy((info->pvTable[ply]) + ply + 1, (info->pvTable[ply + 1]) + ply + 1, info->pvTableLength[ply + 1] * sizeof(Move));
				info->pvTableLength[ply] = info->pvTableLength[ply + 1] + 1;
			}
			alpha = eval;
			bestMove = current;
			
			if (eval >= beta)
			{
				saveTT(pos, current, beta, TT_CUT, depth, ply);
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
		}
	}
	
	if (legalMoves == 0)
	{
		// When no more legal moves, the pv does not exist
		info->pvTableLength[ply] = 0;
		if (inCheck)
		{
			return -INF + ply;
		}
		return 0;
	}
	
	saveTT(pos, bestMove, alpha, nodeBound, depth, ply);
	return alpha;
}

int quiescence(int alpha, int beta, int depth, GameState *pos, SearchInfo *info)
{
	MoveList moveList;
	int size, i, legal;
	
	info->nodes++;
	
	if(( info->nodes & 2047 ) == 0)
	{
		checkTimeLeft(info);
	}

	int eval = evaluation(pos);
	
	if (info->ply > MAX_PLY)
	{
		return evaluation(pos);
	}
	
	if (eval >= beta)
	{
		return beta;
	}

	#ifdef DELTA_PRUNING
	// Delta pruning while not in check
	int BIG_DELTA = 900;
	if (eval < alpha - BIG_DELTA && !isInCheck(pos))
		return alpha;
	#endif

	if (eval > alpha)
	{
		alpha = eval;
	}
	
	moveList = generateMoves(pos, &size);
	scoreMoves(&moveList, pos, 0, info);
	
	for (i = 0; i < size; i++)
	{
		pickMove(&moveList, i);
		Move current = moveList.list[i];
		if (GET_MOVE_CAPTURED(current) == 0 && GET_MOVE_PROMOTION(current) == 0)
		{
			continue;
		}

		GameState newState = playMove(pos, current, &legal);
		
		if (!legal)
			continue;
		
		info->ply++;
		eval = -quiescence(-beta, -alpha, depth, &newState, info);
		info->ply--;
		
		if (info->stopped)
			return 0;
	
		if (eval > alpha)
		{
			alpha = eval;
			if (eval >= beta)
			{
				return beta;
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

void search(GameState *pos, SearchInfo *rootInfo)
{
	int bestScore;
	Move bestMove = 0;
	int alpha = -INF;
	int beta = INF;
	int searchDepth = rootInfo->depth;
	double start, finish;
	
	// Clear information for rootInfo. Will have to do this for ID upon each depth
	start = omp_get_wtime();
	rootInfo->nodes = 0ULL;
	rootInfo->ply = 0;
	memset(rootInfo->killerMoves, 0, sizeof(rootInfo->killerMoves));
	memset(rootInfo->history, 0, sizeof(rootInfo->history));
	memset(rootInfo->pvTable, 0, sizeof(rootInfo->pvTable));
	memset(rootInfo->pvTableLength, 0, sizeof(rootInfo->pvTableLength));
	
	for (int ID = 1; ID <= searchDepth; ID++)
	{
		rootInfo->depth = ID;
		followingPV = 1;
		
		bestScore = negaMax(alpha, beta, ID, pos, rootInfo, 1);

		if (rootInfo->stopped == 1)
			break;

		#ifdef ASPIRATION_WINDOW
		if (bestScore <= alpha || bestScore >= beta)
		{
			alpha = -INF;
			beta = INF;
			ID--;
			continue;
		}
		
		alpha = MAX(bestScore - 50, -INF);
		beta  = MIN(bestScore + 50,  INF);
		#endif

		bestMove = rootInfo->pvTable[0][0];

		// After searching all possible moves, compile stats
		finish = omp_get_wtime() + 0.0001;
		rootInfo->ms = (unsigned int)((finish - start) * 1000);
		rootInfo->nps = (unsigned int)(rootInfo->nodes / (finish - start));
		rootInfo->bestScore = bestScore;
		
		int mated = 0;
		if (rootInfo->bestScore > CHECKMATE)
		{
			rootInfo->bestScore = (INF - rootInfo->bestScore) / 2 + 1;
			mated = 1;
		}
		else if (rootInfo->bestScore < -CHECKMATE)
		{
			rootInfo->bestScore = (-INF - rootInfo->bestScore) / 2;
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
			if (i >= ID || rootInfo->pvTable[0][i] == 0)
				break;
			printf(" ");
			printMove((rootInfo->pvTable[0][i]));
		}
		printf("\n");
		
	}
	// Print the best move
	printf("bestmove ");
	printMove(bestMove);
	printf("\n");
}

int GetTimeMs()
{ 
#ifdef WIN32
	return GetTickCount();
#else
	struct timeval t;
	gettimeofday(&t, NULL);
	return t.tv_sec*1000 + t.tv_usec/1000;
#endif
}


// http://home.arcor.de/dreamlike/chess/
int InputWaiting()
{
#ifndef WIN32
	fd_set readfds;
	struct timeval tv;
	FD_ZERO (&readfds);
	FD_SET (fileno(stdin), &readfds);
	tv.tv_sec=0; tv.tv_usec=0;
	select(16, &readfds, 0, 0, &tv);

	return (FD_ISSET(fileno(stdin), &readfds));
#else
	static int init = 0, pipe;
	static HANDLE inh;
	DWORD dw;

	if (!init) {
		init = 1;
		inh = GetStdHandle(STD_INPUT_HANDLE);
		pipe = !GetConsoleMode(inh, &dw);
		if (!pipe) {
			SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT|ENABLE_WINDOW_INPUT));
			FlushConsoleInputBuffer(inh);
		}
	}
	if (pipe) {
		if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
		return dw;
	} else {
		GetNumberOfConsoleInputEvents(inh, &dw);
		return dw <= 1 ? 0 : dw;
	}
#endif
}

void ReadInput(SearchInfo *info)
{
	int bytes;
	char input[256] = "", *endc;

	if (InputWaiting())
	{
		info->stopped = 1;
		do {
			bytes=read(fileno(stdin),input,256);
		} while (bytes<0);
		endc = strchr(input,'\n');
		if (endc) *endc=0;

		if (strlen(input) > 0) {
			if (!strncmp(input, "quit", 4))
			{
				exit(0);
			}
		}
		return;
	}
}
