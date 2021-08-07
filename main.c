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
	for (int i = 0; i < 4096; i++)
   {
      printBitboard(occupancyFromIndex(i, BishopOccupancy[d4]));
      //getchar();
   }
		//printBitboard(PawnAttacks[1][d4]);
      
	
	return 0;
}