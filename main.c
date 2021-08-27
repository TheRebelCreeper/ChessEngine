#include <stdio.h>
#include <stdlib.h>
#include "bitboard.h"
#include "position.h"
#include "movegen.h"

#define TEST_POSITION_1 "r4r1k/pp3pR1/2p5/2b1p3/4P2p/NP1P1P2/1PP2K2/3q2Q1 w - - 5 34"
#define TEST_POSITION_2 "r2qkb1r/pp2pp1p/2p3p1/5b2/Q2nnB1N/3B4/PP3PPP/RN3RK1 w kq - 0 11"
#define TEST_POSITION_3 "r1bqkbnr/ppp2pp1/2n4p/3pp1B1/2PP4/5N2/PP2PPPP/RN1QKB1R w KQkq - 0 1"
#define TEST_POSITION_4 "r1bqkbnr/ppp2pp1/2n4p/3pp1B1/2PP3P/5N2/PP2PPP1/RN1QKB1R b KQkq - 0 1"

void exampleMagicBitboard()
{
	U64 blocker = 0ULL;
	set_square(blocker, c6);
	set_square(blocker, f3);
	set_square(blocker, g6);
	set_square(blocker, e2);
	printBitboard(getBishopAttacks(e4, blocker));
	printBitboard(getRookAttacks(e6, blocker));
	printBitboard(getQueenAttacks(e4, blocker));
}

void testIsAttacked()
{
	//printf("Is c6 attacked by BLACK: "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(isSquareAttacked(c6, WHITE)));
	U64 attacked = 0ULL;
	for (int i = 0; i < 2; i++)
	{
		for (int square = 0; square < 64; square++)
		{
			if (isSquareAttacked(square, i))
				set_square(attacked, square);
		}
		printf("Attacked by %s\n", (i == 0) ? "White" : "Black");
		printBitboard(attacked);
		attacked = 0ULL;
	}
}

int main()
{
	initAttacks();
	
	initStartingPosition();
	//printBoard();
	
	loadFEN(TEST_POSITION_3);
	printBoard();
	generateMoves(state);
	//testIsAttacked();
	//exampleMagicBitboard();
	
	return 0;
}
