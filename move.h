#ifndef MOVE_H
#define MOVE_H

typedef struct move
{
	int src;
	int dst;
	int piece;
} Move;

int compareMoves(Move m1, Move m2);

#endif