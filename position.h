#ifndef POSITION_H
#define POSITION_H

#include "bitboard.h"

#define DELIMS "/ "
#define BOTH 2

#define WHITE_OO 8
#define WHITE_OOO 4
#define BLACK_OO 2
#define BLACK_OOO 1

#define STARTING_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

typedef struct gameState
{
	U64 pieceBitboards[12];
	U64 occupancies[3];
	int turn;
	int castlingRights;
	int enpassantSquare;
	int halfMoveClock;
	int fullMove;
} GameState;

extern GameState state;

extern char *pieceNotation[12];

enum
{
	P, N, B, R, Q, K, p, n, b, r, q, k, NO_PIECE
};

void loadFEN(GameState *state, char *fen);
void setOccupancies(GameState *state);
void initStartingPosition();
char isSquareAttacked(GameState state, int square, int byColor);

void printBoard(GameState state);

#endif
