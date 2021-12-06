#ifndef EVALUATION_H
#define EVALUATION_H

#include "position.h"

#define INF 1000000
#define CHECKMATE 999900

static const int pieceValue[12] = {100, 310, 330, 500, 900, 10000, -100, -300, -325, -500, -900, -10000};

static const int pawnScore[64] = 
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

static const int knightScore[64] =
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

static const int bishopScore[64] =
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

static const int rookScore[64] =
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

static const int kingScore[64] =
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

static const int mirroredSquare[65] =
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

static const int nnue_pieces[12] =
{
	6, 5, 4, 3, 2, 1, 12, 11, 10, 9, 8, 7
};

void printEvaluation(int score);
int nnue_eval(GameState *pos);
int evaluation(GameState *pos);

#endif