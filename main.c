#include <stdio.h>
#include <stdlib.h>
#include "bitboard.h"

int main()
{
	//U64 board = (Rank4 | Rank5) & (FileD | FileE);
	//set_square(board, e4);
	//clear_square(board, e4);
	initLeapers();
	initSliders();
	//for (int i = 0; i < 64; i++)
		//printBitboard(PawnAttacks[1][d4]);
	return 0;
}