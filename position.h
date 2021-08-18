#ifndef POSITION_H
#define POSITION_H

#include "bitboard.h"

#define DELIMS "/ "

#define WHITE_OO 8
#define WHITE_OOO 4
#define BLACK_OO 2
#define BLACK_OOO 1

extern int enpessantSquare;
extern int turn;
extern char castlingRights;
extern U64 PIECE_BITBOARDS[12];

enum
{
	P, N, B, R, Q, K, p, n, b, r, q, k, NO_PIECE
};

void loadFEN(char *fen);
void initStartingPosition();
U64 getBlackPieces();
U64 getWhitePieces();
U64 getAllPieces();

void printBoard();

#endif