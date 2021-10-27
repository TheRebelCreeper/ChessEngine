#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "bitboard.h"
#include "movegen.h"
#include "search.h"
#include "uci.h"
#include "perft.h"

#define TEST_POSITION_DRAW_50 "k7/5R2/1K6/8/8/8/8/8 b - - 99 1"
#define TEST_POSITION_KPENDGAME "4k3/8/3K4/3P4/8/8/8/8 w - - 1 2"
#define TEST_POSITION_M5 "8/3K4/5k2/8/8/3Q4/8/8 w - - 7 9"
#define TEST_POSITION_M2 "r2qkbnr/ppp2ppp/2np4/4N3/2B1P3/2N5/PPPP1PPP/R1BbK2R w KQkq - 0 6"
#define TEST_POSITION_M1 "3r1r2/pQ2pp1N/3pk1p1/2pNb2n/P7/1P4P1/7P/R1B1KR2 w Q - 6 22"

int main(int argc, char *argv[])
{
	initAttacks();
	
	#ifdef DEBUG
	moveGeneratorValidator();
	#else
	uciLoop();
	#endif
	return 0;
}
