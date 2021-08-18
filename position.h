#ifndef POSITION_H
#define POSITION_H

#include "bitboard.h"

extern U64 WHITE_PAWNS;
extern U64 WHITE_KNIGHTS;
extern U64 WHITE_BISHOPS;
extern U64 WHITE_ROOKS;
extern U64 WHITE_QUEENS;
extern U64 WHITE_KINGS;
extern U64 WHITE_PIECES;

extern U64 BLACK_PAWNS;
extern U64 BLACK_KNIGHTS;
extern U64 BLACK_BISHOPS;
extern U64 BLACK_ROOKS;
extern U64 BLACK_QUEENS;
extern U64 BLACK_KINGS;
extern U64 BLACK_PIECES;

enum
{
	P, N, B, R, Q, K, p, n, b, r, q, k, NO_PIECE
};

void initStartingPosition();
U64 getBlackPieces();
U64 getWhitePieces();
U64 getAllPieces();

void printBoard();

#endif