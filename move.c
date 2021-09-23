#include "move.h"
int compareMoves(Move m1, Move m2)
{
	    return (m1.piece == m2.piece) && (m1.src == m2.src) && (m1.dst == m2.dst);
}
