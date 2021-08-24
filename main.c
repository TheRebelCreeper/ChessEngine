#include <stdio.h>
#include <stdlib.h>
#include "bitboard.h"
#include "position.h"

#define TEST_POSITION_1 "r4r1k/pp3pR1/2p5/2b1p3/4P2p/NP1P1P2/1PP2K2/3q2Q1 w - - 5 34"
#define TEST_POSITION_2 "k7/3n1P2/2q5/4n3/2Q3N1/3K4/5B2/8 w - - 0 1"

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

void testIsAttacked()
{
	printf("Is d3 attacked by BLACK: %d\n", isSquareAttacked(d3, BLACK));
	printf("Is c4 attacked by BLACK: %d\n", isSquareAttacked(c4, BLACK));
	printf("Is c5 attacked by BLACK: %d\n", isSquareAttacked(c5, BLACK));
	printf("Is c6 attacked by BLACK: %d\n", isSquareAttacked(c6, BLACK));
	printf("Is f2 attacked by BLACK: %d\n", isSquareAttacked(f2, BLACK));
	printf("Is a1 attacked by BLACK: %d\n", isSquareAttacked(a1, BLACK));
	printf("Is g8 attacked by WHITE: %d\n", isSquareAttacked(g8, WHITE));
}

int main()
{
	initAttacks();
	
	initStartingPosition();
	printBoard();
	loadFEN(TEST_POSITION_1);
	printBoard();
	//printBitboard(getAllPieces());
	//testIsAttacked();
	//exampleMagicBitboard();
	
	return 0;
}
