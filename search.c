#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "movegen.h"
#include "move.h"
#include "search.h"
#include "evaluation.h"

int negaMax(int alpha, int beta, int depth, GameState *pos)
{
	MoveList moveList;
	int size, i;
	int bestScore;
	int moveScores[256];
	
	memset(moveScores, 0, 256 * sizeof(int));
	moveList = generateMoves(pos, &size);
	
	if (size == 0)
	{
		int offset = 6 * pos->turn;
		int kingLocation = getFirstBitSquare(pos->pieceBitboards[K + offset]);
		if (isSquareAttacked(pos, kingLocation, (pos->turn == WHITE) ? BLACK : WHITE))
			return -CHECKMATE;
		return 0;
	}
	
	if (depth == 0)
	{
		return evaluation(pos);
	}
	
	for (i = 0; i < moveList.nextOpen; i++)
	{
		Move current = moveList.list[i];
		if (current.legal == 1)
		{
			GameState newState = playMove(pos, moveList.list[i]);
			moveScores[i] = -negaMax(-beta, -alpha, depth - 1, &newState);
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
	return alpha;
}

Move search(int depth, GameState *pos, int *score)
{
	MoveList moveList;
	int size, i;
	int bestScore, bestIndex;
	int moveScores[256];
	
	memset(moveScores, 0, 256 * sizeof(int));
	moveList = generateMoves(pos, &size);
	
	bestIndex = 0;
	bestScore = -CHECKMATE;
	for (i = 0; i < moveList.nextOpen; i++)
	{
		Move current = moveList.list[i];
		if (current.legal == 1)
		{
			GameState newState = playMove(pos, moveList.list[i]);
			moveScores[i] = -negaMax(-CHECKMATE, CHECKMATE, depth - 1, &newState);
			if (moveScores[i] > bestScore)
			{
				bestIndex = i;
				bestScore = moveScores[i];
			}
		}
	}
	*score = bestScore;
	return moveList.list[bestIndex];
}
