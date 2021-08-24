#ifndef POSITION_H
#define POSITION_H

#include "bitboard.h"

#define DELIMS "/ "
//#define UNICODE_PIECES

#define WHITE_OO 8
#define WHITE_OOO 4
#define BLACK_OO 2
#define BLACK_OOO 1

#define STARTING_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

struct GameState
{
	U64 pieceBitboards[12];
	int turn;
	int castlingRights;
	int enpessantSquare;
	int halfMoveClock;
	int fullMove;
};

extern struct GameState state;

enum
{
	P, N, B, R, Q, K, p, n, b, r, q, k, NO_PIECE
};

void loadFEN(char *fen);
void initStartingPosition();
U64 getBlackPieces();
U64 getWhitePieces();
U64 getAllPieces();
char isSquareAttacked(int square, int byColor);

void printBoard();

#endif
