#include <stdio.h>
#include <stdlib.h>
#include "position.h"
#include "evaluation.h"

int pieceValue[12] = {100, 310, 330, 500, 900, 10000, -100, -300, -325, -500, -900, -10000};
int mirroredSquare[65] = 
{
	a8, b8, c8, d8, e8, f8, g8, h8,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a1, b1, c1, d1, e1, f1, g1, h1, none
};

const int pawnScore[64] = 
{
     0,   0,   0,   0,   0,   0,   0,   0,
    50,  50,  50,  60,  60,  50,  50,  50,
    20,  20,  20,  30,  30,  20,  20,  20,
    10,  10,  10,  25,  25,  10,  10,  10,
     5,   5,  10,  20,  20,   0,   0,   0,
     5,   0,   0,   0,   0, -10,  -5,   5,
     5,  10,  10, -20, -20,  10,  10,   5,
     0,   0,   0,   0,   0,   0,   0,   0
};

const int knightScore[64] = 
{
    -50,   0,   0,   0,   0,   0,   0, -50,
    -25,   0,   0,   0,   0,   0,   0, -25,
    -25,   5,  10,  15,  15,  10,   5, -25,
    -25,  10,  15,  20,  20,  15,  10, -25,
    -25,  10,  15,  20,  20,  15,  10, -25,
    -25,   5,  10,  15,  15,  10,   5, -25,
    -25,   0,   0,   5,   5,   0,   0, -25,
    -50, -10,   0,   0,   0,   0, -10, -50
};

const int bishopScore[64] = 
{
    -20,-10,-10,-10,-10,-10,-10,-20,
	-10,  0,  0,  0,  0,  0,  0,-10,
	-10,  0,  5, 10, 10,  5,  0,-10,
	-10,  5,  5, 10, 10,  5,  5,-10,
	-10,  0, 10, 10, 10, 10,  0,-10,
	-10, 10, 10, 10, 10, 10, 10,-10,
	-10,  5,  0,  0,  0,  0,  5,-10,
	-20,-10,-10,-10,-10,-10,-10,-20

};

const int rookScore[64] =
{
    25,  25,  25,  25,  25,  25,  25,  25,
    50,  50,  50,  50,  50,  50,  50,  50,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,   5,  20,  20,   5,   0,   0

};

const int kingScore[64] = 
{
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   5,   5,   5,   5,   0,   0,
     0,   5,   5,  10,  10,   5,   5,   0,
     0,   5,  10,  20,  20,  10,   5,   0,
     0,   5,  10,  20,  20,  10,   5,   0,
     0,   0,   5,  10,  10,   5,   0,   0,
     0,   5,   5,  -5,  -5,   0,   5,   0,
     0,   0,   5,   0, -15,  -5,  10,   0
};

int materialCount(GameState *pos)
{
	int i;
	int score = 0;
	U64 pieceBB;
	for (i = P; i <= k; i++)
	{
		pieceBB = pos->pieceBitboards[i];
		while (pieceBB)
		{
			int sqr = getFirstBitSquare(pieceBB);
			score += pieceValue[i];
			if (i == P)
				score += pawnScore[mirroredSquare[sqr]];
			else if (i == p)
				score -= pawnScore[sqr];
			else if (i == N)
				score += knightScore[mirroredSquare[sqr]];
			else if (i == n)
				score -= knightScore[sqr];
			else if (i == B)
				score += bishopScore[mirroredSquare[sqr]];
			else if (i == b)
				score -= bishopScore[sqr];
			else if (i == R)
				score += rookScore[mirroredSquare[sqr]];
			else if (i == r)
				score -= rookScore[sqr];
			else if (i == K)
				score += kingScore[mirroredSquare[sqr]];
			else if (i == k)
				score -= kingScore[sqr];
			clear_lsb(pieceBB);
		}
	}
	return (pos->turn == WHITE) ? score : -score;
}

int evaluation(GameState *pos)
{
	int score = 0;
	score += materialCount(pos);
	return score;
}

void printEvaluation(int score)
{
	int mated = 0;
	if (score > MAX_PLY_CHECKMATE)
	{
		score = CHECKMATE - score;
		mated = 1;
	}
	else if (score < -MAX_PLY_CHECKMATE)
	{
		score = -CHECKMATE - score;
		mated = 1;
	}
	printf("Eval: %s%d\n", (mated) ? "#" : "", score);
}