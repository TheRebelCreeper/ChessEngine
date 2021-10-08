#include <stdio.h>
#include <stdlib.h>
#include "bitboard.h"
#include "position.h"
#include "movegen.h"
#include "move.h"
#include "list.h"

#define TEST_POSITION_1 "r4r1k/pp3pR1/2p5/2b1p3/4P2p/NP1P1P2/1PP2K2/3q2Q1 w - - 5 34"
#define TEST_POSITION_2 "r2qkb1r/pp2pp1p/2p3p1/5b2/Q2nnB1N/3B4/PP3PPP/RN3RK1 w kq - 0 11"
#define TEST_POSITION_3 "r1bqkbnr/ppp2pp1/2n4p/3pp1B1/2PP4/5N2/PP2PPPP/RN1QKB1R w KQkq - 0 1"
#define TEST_POSITION_4 "r1bqkbnr/ppp2pp1/2n4p/3pp1B1/2PP3P/5N2/PP2PPP1/RN1QKB1R b KQkq - 0 1"
#define TEST_POSITION_PROMOTION "3r4/2P5/K7/8/8/8/5pk1/8 w - - 0 1"
#define TEST_POSITION_EP "rnbqkbnr/ppp2ppp/4p3/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3"
#define TEST_POSITION_CASTLES "r3k2r/pppp1ppp/2n2n2/1Bb1p3/4P3/5N2/PPPP1PPP/R3K2R b KQkq - 0 1"
#define TEST_POSITION_STALEMATE "2k5/2P5/2K5/8/8/8/8/8 b - - 0 1"
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

int main()
{
	Node *moveList = NULL;
	int size;
	initAttacks();
	initStartingPosition();
	//loadFEN(&state, "8/8/7p/5K1k/6Pp/7P/8/8 b - g3 0 1");
	printBoard(state);
	size = perft(6, state);
	printf("Number of legal moves: %d\n", size);
	
	return 0;
}
