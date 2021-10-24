#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "bitboard.h"
#include "movegen.h"
#include "search.h"
#include "uci.h"

#define PERFT_POSITION_1 "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define PERFT_POSITION_2 "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
#define PERFT_POSITION_3 "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"
#define PERFT_POSITION_4 "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"
#define PERFT_POSITION_4B "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1"
#define PERFT_POSITION_5 "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"
#define PERFT_POSITION_6 "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"
#define TEST_POSITION_DRAW_50 "k7/5R2/1K6/8/8/8/8/8 b - - 99 1"
#define TEST_POSITION_KPENDGAME "4k3/8/3K4/3P4/8/8/8/8 w - - 1 2"
#define TEST_POSITION_M5 "8/3K4/5k2/8/8/3Q4/8/8 w - - 7 9"
#define TEST_POSITION_M2 "r2qkbnr/ppp2ppp/2np4/4N3/2B1P3/2N5/PPPP1PPP/R1BbK2R w KQkq - 0 6"
#define TEST_POSITION_M1 "3r1r2/pQ2pp1N/3pk1p1/2pNb2n/P7/1P4P1/7P/R1B1KR2 w Q - 6 22"

int main(int argc, char *argv[])
{
	char moveInput[256];
	int size;
	int score;
	MoveList moveList;
	Move bestMove;
	initAttacks();
	
	#ifdef DEBUG
	int depth = atoi(argv[1]);
	double start, finish;

	// Game Loop
	
	while (1)
	{
		int size;
		printBoard(state);
		moveList = generateMoves(&state, &size);
		
		if (size == 0)
		{
			printf("Game Over!\n");
			return 0;
		}
		start = omp_get_wtime();
		bestMove = search(depth, &state, &score);
		finish = omp_get_wtime();
		
		printEvaluation(score);
		printf("%s%s-%s\n", pieceNotation[bestMove.piece], squareNames[bestMove.src], squareNames[bestMove.dst]);
		printf("Finished search in %f seconds\n\n", finish - start);
		
		while (1)
		{
			printf("Enter move: ");
			if (fgets(moveInput, 256, stdin) == NULL)
			{
				exit(0);
			}
			if (strncmp(moveInput, "quit", 4) == 0)
			{
				return 0;
			}
			int idx = parseMove(moveInput, &moveList);
			if (idx != -1)
			{
				state = playMove(&state, moveList.list[idx]);
				break;
			}
			else
			{
				printf("Illegal move\n");
			}
		}
	}
	#else
	uciLoop();
	#endif
	return 0;
}
