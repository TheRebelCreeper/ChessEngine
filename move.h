#ifndef MOVE_H
#define MOVE_H

#define OO_SPECIAL 1
#define OOO_SPECIAL 2
#define EN_PASSANT_SPECIAL 5
#define NO_SPECIAL 0

#define IS_CHECK 2
#define IS_CAPTURE 1

#include "position.h"

/*
	There are special moves for kings and pawns only
	For kings, 1 = O-O, 2 = O-O-O
	For pawns, 4 = Q, 3 = R, 2 = B, 1 = N, 5 = EP
*/
typedef struct move
{
	int src;
	int dst;
	int piece;
	int special;
	int epSquare;
	int legal;
	int prop;
} Move;

int compareMoves(const void * a, const void * b);
Move createMove(int piece, int src, int dst, int special, int epSquare);
GameState playMove(GameState *pos, Move move);

#endif