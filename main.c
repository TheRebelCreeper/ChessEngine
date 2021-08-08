#include <stdio.h>
#include <stdlib.h>
#include "bitboard.h"

void exampleMagicBitboard()
{
	U64 blocker = 0ULL;
	U64 magic = 0x7c0c028f5b34ff76; // Magic for g3
	set_square(blocker, d6);
	set_square(blocker, f2);
	printBitboard(blocker);
	printBitboard(BishopOccupancy[g3]);
	
	int bitLength = countBits(BishopOccupancy[g3]);
	for (int i = 0; i < (1 << bitLength); i++)
	{
		U64 occupancy = occupancyFromIndex(i, BishopOccupancy[g3]);
		
		int mIndex = (occupancy * magic) >> (64 - countBits(BishopOccupancy[g3]));
		BishopAttacks[g3][mIndex] = generateBishopAttacks(2, 6, occupancy);
	}

	U64 attacks = blocker;
	attacks &= BishopOccupancy[g3];
	attacks *= magic;
	attacks >>= 64 - countBits(BishopOccupancy[g3]);
	printBitboard(BishopAttacks[g3][attacks]);
}

int main()
{
	initLeapers();
	initSliders();
	
	exampleMagicBitboard();
      
	
	return 0;
}