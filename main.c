#include <stdio.h>
#include <stdlib.h>
#include "bitboard.h"
#include "position.h"
#include "movegen.h"

#define TEST_POSITION_1 "r4r1k/pp3pR1/2p5/2b1p3/4P2p/NP1P1P2/1PP2K2/3q2Q1 w - - 5 34"
#define TEST_POSITION_2 "r2qkb1r/pp2pp1p/2p3p1/5b2/Q2nnB1N/3B4/PP3PPP/RN3RK1 w kq - 0 11"
#define TEST_POSITION_3 "r1bqkbnr/ppp2pp1/2n4p/3pp1B1/2PP4/5N2/PP2PPPP/RN1QKB1R w KQkq - 0 1"
#define TEST_POSITION_4 "r1bqkbnr/ppp2pp1/2n4p/3pp1B1/2PP3P/5N2/PP2PPP1/RN1QKB1R b KQkq - 0 1"
#define TEST_POSITION_PROMOTION "3r4/2P5/K7/8/8/8/5pk1/8 w - - 0 1"
#define TEST_POSITION_EP "rnbqkbnr/ppp2ppp/4p3/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3"
#define TEST_POSITION_CASTLES "r3k2r/pppp1ppp/2n2n2/1Bb1p3/4P3/5N2/PPPP1PPP/R3K2R b KQkq - 0 1"

void testIsAttacked()
{
	//printf("Is c6 attacked by BLACK: "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(isSquareAttacked(c6, WHITE)));
	U64 attacked = 0ULL;
	for (int i = 0; i < 2; i++)
	{
		for (int square = 0; square < 64; square++)
		{
			if (isSquareAttacked(state, square, i))
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
	//loadFEN(TEST_POSITION_CASTLES);
	printBoard(state);
	generateMoves(state);
	//testIsAttacked();
	
	return 0;
}
