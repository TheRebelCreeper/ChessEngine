#include <stdio.h>
#include <stdlib.h>
#include "position.h"
#include "evaluation.h"

int pieceValue[12] = {100, 300, 325, 500, 900, 10000, -100, -300, -325, -500, -900, -10000};

int materialCount(GameState *pos)
{
	int i;
	int score = 0;
	for (i = P; i <= k; i++)
	{
		score += (pieceValue[i] * countBits(pos->pieceBitboards[i]));
	}
	return (pos->turn == WHITE) ? score : -score;
}

int evaluation(GameState *pos)
{
	int i;
	int score = 0;
	score += materialCount(pos);
	return score;
}