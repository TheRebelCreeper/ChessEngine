#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "bitboard.h"
#include "position.h"
#include "movegen.h"
#include "move.h"
#include "evaluation.h"
#include "search.h"

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

U64 perft(int depth, GameState *pos)
{
	MoveList moveList;
	int size;
	U64 sum = 0;
	if (depth == 0)
	{
		return 1ULL;
	}
	moveList = generateMoves(pos, &size);
	if (depth == 1)
	{
		return (U64)size;
	}
	
	for (int i = 0; i < moveList.nextOpen; i++)
	{
		if (moveList.list[i].legal == 1)
		{
			GameState newState = playMove(pos, moveList.list[i]);
			sum += perft(depth - 1, &newState);
		}
	}
	return sum;
}

U64 perftDivide(int depth, GameState *pos)
{
	MoveList moveList;
	int size, i;
	U64 sum = 0;
	
	if (depth == 0)
	{
		return 1ULL;
	}
	
	moveList = generateMoves(pos, &size);
	printf("Perft results for depth %d:\n", depth);

	#pragma omp parallel for num_threads(12) shared(moveList) reduction(+:sum)
	for (i = 0; i < moveList.nextOpen; i++)
	{
		Move current = moveList.list[i];
		if (current.legal == 1)
		{
			GameState newState = playMove(pos, moveList.list[i]);
			U64 res = perft(depth - 1, &newState);
			sum += res;
			if (current.special == NO_SPECIAL || current.special == EN_PASSANT_SPECIAL || current.piece == K || current.piece == k)
			{
				printf("%s%s", squareNames[current.src], squareNames[current.dst]);
			}
			else
			{
				printf("%s%s=%s", squareNames[current.src], squareNames[current.dst], pieceNotation[current.special]);
			}
			printf(": %llu\n", res);
		}
	}
	
	return sum;
}

void runPerft(int depth)
{
	U64 size;
	double start, finish;
	start = omp_get_wtime();
	size = perftDivide(depth, &state);
	printf("Perft Nodes: %llu\n\n", size);
	finish = omp_get_wtime();
	printf("Finished perft in %f seconds\n", finish - start);
	printf("NPS: %f\n", size / (finish - start));
}

int main(int argc, char *argv[])
{
	char moveInput[5];

	int score;
	MoveList moveList;
	Move bestMove;
	initAttacks();
	initStartingPosition();
	loadFEN(&state, PERFT_POSITION_6);
	
	int depth = atoi(argv[1]);
	double start, finish;
	
	runPerft(depth);
	return 0;

	// Game Loop
	
	while (1)
	{
		int size, src, dst, found = 0;
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
		
		do
		{
			char c;
			printf("Enter move: ");
			if (fgets(moveInput, 5, stdin) == NULL) exit(0);
			while (((c = getchar()) != EOF) && (c != '\n'));
			if (strcmp(moveInput, "quit") == 0)
				exit(0);
			
		} while (!found);
	}

	return 0;
}
