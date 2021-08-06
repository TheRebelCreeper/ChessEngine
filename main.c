#include <stdio.h>
#include <stdlib.h>
#include "bitboard.h"

int main()
{
	U64 blocker = 0ULL;
	set_square(blocker, b4);
	set_square(blocker, f7);
	set_square(blocker, g4);
	set_square(blocker, f3);
	//clear_square(board, e4);
	initLeapers();
	initSliders();
	//for (int i = 0; i < 64; i++)
		//printBitboard(PawnAttacks[1][d4]);
	printBitboard(generateRookAttacks(3, 5, blocker));
	return 0;
}