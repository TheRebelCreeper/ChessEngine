#include "move.h"
int compareMoves(Move m1, Move m2)
{
	    return (m1.piece == m2.piece) && (m1.src == m2.src) && (m1.dst == m2.dst);
}

/*
	There are special moves for kings and pawns only
	For kings, 1 = O-O, 2 = O-O-O
	For pawns, 5 = Q, 4 = R, 3 = B, 2 = N, 1 = EP
*/
Move createMove(int piece, int src, int dst, int special)
{
	Move newMove;
	newMove.piece = piece;
	newMove.src = src;
	newMove.dst = dst;
	newMove.special = special;
	return newMove;
}
