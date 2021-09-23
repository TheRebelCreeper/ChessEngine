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

struct GameState
{
	U64 pieceBitboards[12];
	U64 occupancies[3];
	int turn;
	int castlingRights;
	int enpassantSquare;
	int halfMoveClock;
	int fullMove;
};

extern struct GameState state;

extern char *pieceNotation[12];

enum
{
	P, N, B, R, Q, K, p, n, b, r, q, k, NO_PIECE
};

void loadFEN(struct GameState *state, char *fen);
void setOccupanies(struct GameState *state);
void initStartingPosition();
char isSquareAttacked(struct GameState state, int square, int byColor);

void printBoard(struct GameState state);

#endif
