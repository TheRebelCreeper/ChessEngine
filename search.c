#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#endif

#include "movegen.h"
#include "move.h"
#include "search.h"

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

// This isn't perfect solution. Has issues if approachin 50 move rule with overflow.
// Giving each position a copy of posHistory and historyIndex should fix because
// The history index could be reset on captures, or making array longer
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
	// return reps >= 2; This handles true threefold instead of single rep
	return reps != 0;
}

inline int okToReduce(Move move, GameState *parent, GameState *child)
{
	return (GET_MOVE_CAPTURED(move) == 0 && GET_MOVE_PROMOTION(move) == 0 && isInCheck(parent) == 0 && isInCheck(child) == 0);
}

int quiescence(int alpha, int beta, int depth, GameState *pos, SearchInfo *info)
{
	MoveList moveList;
	int size, i, legal;
	
	if(( info->nodes & 2047 ) == 0)
	{
		checkTimeLeft(info);
	}

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

			if (info->stopped)
				return 0;

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
	int movesSearched = 0;
	int ply = info->ply;
	int inCheck = isInCheck(pos);
	info->pvTableLength[ply] = depth;
	
	// Repetition only possible if halfmove is > 4
	if (ply && pos->halfMoveClock > 4 && is_repetition(pos))
	{
		return 0;
	}

	// Enter quiescence if not in check
	if (depth <= 0 && !inCheck)
	{
		return quiescence(alpha, beta, info->depth, pos, info);
	}

	if (ply >= MAX_PLY)
	{
		return evaluation(pos);
	}

	if(( info->nodes & 2047 ) == 0)
	{
		checkTimeLeft(info);
	}

	// Disabled for now. Might be good after TT
	#ifdef CHECK_EXTENTIONS
	if (inCheck)
		depth++;
	#endif
	
	// Null move pruning
	if (info->stopped == 0 && nullMove && ply && !inCheck && depth >= 3 && countBits(pos->occupancies[BOTH]) > 10)
	{
		GameState newPos;
		memcpy(&newPos, pos, sizeof(GameState));
		// Make null move by switching side
		newPos.turn ^= 1;
		newPos.enpassantSquare = none;
		info->ply++;
		eval = -negaMax(-beta, -beta + 1, depth - 3, 0, &newPos, info);
		info->ply--;
		if (eval >= beta)
		{
			return beta;
		}
	}

	moveList = generateMoves(pos, &size);
	scoreMoves(&moveList, pos, depth, info);
	
	for (i = 0; i < size; i++)
	{
		pickMove(&moveList, i);
		Move current = moveList.list[i];
		GameState newState = playMove(pos, current, &legal);

		// Increment history index to next depth
		
		if (legal == 1)
		{
			// Save the current move into history
			historyIndex++;
			info->ply++;
			posHistory[historyIndex] = newState.key;
			found = 1;
			
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
			
			// Unmake move by removing current move from history
			info->ply--;
			historyIndex--;
			

			if (info->stopped)
				return 0;

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
		if (inCheck)
		{
			return -CHECKMATE + ply;
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

void search(GameState *pos, SearchInfo *rootInfo)
{
	int bestScore;
	int bestMove = 0;
	int alpha = -CHECKMATE;
	int beta = CHECKMATE;
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
		bestScore = negaMax(alpha, beta, ID, 1, pos, rootInfo);

		if (rootInfo->stopped == 1)
			break;

		#ifdef ASPIRATION_WINDOW
		if (bestScore <= alpha || bestScore >= beta)
		{
			alpha = -CHECKMATE;
			beta = CHECKMATE;
			ID--;
			continue;
		}
		
		alpha = bestScore - 50;
		beta = bestScore + 50;
		#endif

		bestMove = rootInfo->pvTable[0][0];

		// After searching all possible moves, compile stats
		finish = omp_get_wtime() + 0.0001;
		rootInfo->ms = (unsigned int)((finish - start) * 1000);
		rootInfo->nps = (unsigned int)(rootInfo->nodes / (finish - start));
		rootInfo->bestScore = bestScore;
		
		int mated = 0;
		if (rootInfo->bestScore > MAX_PLY_CHECKMATE)
		{
			rootInfo->bestScore = (CHECKMATE - rootInfo->bestScore) / 2 + 1;
			mated = 1;
		}
		else if (rootInfo->bestScore < -MAX_PLY_CHECKMATE)
		{
			rootInfo->bestScore = (-CHECKMATE - rootInfo->bestScore) / 2;
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
