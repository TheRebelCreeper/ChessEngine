#include <stdio.h>
#include <stdlib.h>
#include "bitboard.h"

void exampleMagicBitboard()
{
	U64 blocker = 0ULL;
	set_square(blocker, c6);
	set_square(blocker, f3);
	set_square(blocker, g6);
	set_square(blocker, e2);
	printBitboard(getBishopAttack(e4, blocker));
	printBitboard(getRookAttack(e6, blocker));
	
	for (int i = 0; i < 64; i++)
	{
		//printf("0x%llxULL\n", BishopMagic[i]);
	}
}

int main()
{
	initLeapers();
	initSliders();
	
	exampleMagicBitboard();
	
	return 0;
}