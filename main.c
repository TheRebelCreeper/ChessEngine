#include <stdio.h>
#include <stdlib.h>
#include "bitboard.h"

void exampleMagicBitboard()
{
	U64 blocker = 0ULL;
	set_square(blocker, d6);
	printf("0x%llx\n", BishopMagic[f4]);
	printBitboard(getBishopAttack(f4, blocker));
}

int main()
{
	initLeapers();
	initSliders();
	
	exampleMagicBitboard();
	
	return 0;
}