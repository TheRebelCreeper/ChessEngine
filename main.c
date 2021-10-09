#include <stdio.h>
#include <stdlib.h>
#include "bitboard.h"
#include "position.h"
#include "movegen.h"
#include "move.h"
#include "list.h"

#define PERFT_POSITION_1 "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define PERFT_POSITION_2 "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
#define PERFT_POSITION_3 "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"
#define PERFT_POSITION_4 "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"
#define PERFT_POSITION_4B "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1"
#define PERFT_POSITION_5 "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"
#define PERFT_POSITION_6 "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"
#define TEST_POSITION_STALEMATE "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"
#define TEST_POSITION_CHECKMATE "2k1R3/8/2K5/8/8/8/8/8 b - - 0 1"

U64 perft(int depth, GameState state)
{
	Node *moveList = NULL;
	Node *current = NULL;
	int size;
	U64 sum = 0;
	
	if (depth == 0)
	{
		return 1ULL;
	}
	
	moveList = generateMoves(state, &size);
	
	current = moveList;
	while (current != NULL)
	{
		GameState newState = playMove(state, current->move);
		sum += perft(depth - 1, newState);
		current = current->next;
	}
	deleteList(moveList);
	return sum;
}

U64 perftDivide(int depth, GameState state)
{
	Node *moveList = NULL;
	Node *current = NULL;
	int size;
	U64 sum = 0;
	
	if (depth == 0)
	{
		return 1ULL;
	}
	
	moveList = generateMoves(state, &size);
	printf("Perft results for depth %d:\n", depth);
	current = moveList;
	while (current != NULL)
	{
		GameState newState = playMove(state, current->move);
		U64 res = perft(depth - 1, newState);
		sum += res;
		if (current->move.special == NO_SPECIAL || current->move.special == EN_PASSANT_SPECIAL || current->move.piece == K || current->move.piece == k)
		{
			printf("%s%s", squareNames[current->move.src], squareNames[current->move.dst]);
		}
		else
		{
			printf("%s%s=%s", squareNames[current->move.src], squareNames[current->move.dst], pieceNotation[current->move.special]);
		}
		printf(": %d\n", res);
		current = current->next;
	}
	deleteList(moveList);
	return sum;
}

int main(int argc, char *argv[])
{
	U64 size;
	initAttacks();
	initStartingPosition();
	loadFEN(&state, PERFT_POSITION_1);
	printBoard(state);
	
	/*
	Node *moveList = NULL;
	moveList = generateMoves(state, &size);
	printMoveList(moveList, state);
	GameState newState = playMove(state, getNode(moveList, 1)->move);
	printBoard(newState);
	*/
	
	size = perftDivide(atoi(argv[1]), state);
	printf("Perft Nodes: %llu\n", size);
	
	return 0;
}
