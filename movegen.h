#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "bitboard.h"
#include "position.h"

struct Move
{
	int src;
	int dst;
	int piece;
};

void generateMoves(struct GameState position);

#endif