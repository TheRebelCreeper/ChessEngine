#include <stdio.h>
#include <stdlib.h>
#include "bitboard.h"
#include "position.h"

#define TEST_POSITION_1 "r4r1k/pp3pR1/2p5/2b1p3/4P2p/NP1P1P2/1PP2K2/3q2Q1 b - - 5 34"

void exampleMagicBitboard()
{
	U64 blocker = 0ULL;
	set_square(blocker, c6);
	set_square(blocker, f3);
	set_square(blocker, g6);
	set_square(blocker, e2);
	printBitboard(getBishopAttack(e4, blocker));
	printBitboard(getRookAttack(e6, blocker));
	printBitboard(getQueenAttack(e4, blocker));
}

int main()
{
	initAttacks();
	
	//initStartingPosition();
	//printBoard();
	loadFEN(TEST_POSITION_1);
	printBoard();
	
	//exampleMagicBitboard();
	
	return 0;
}
