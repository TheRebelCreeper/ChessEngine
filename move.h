#ifndef MOVE_H
#define MOVE_H

#define OO_SPECIAL 1
#define OOO_SPECIAL 2
#define EN_PASSANT_SPECIAL 5
#define NO_SPECIAL 0

#define IS_CHECK 8
#define IS_CAPTURE 0x8000000
#define IS_PROMOTION 0x10000000
#define IS_EN_PASSANT 0x20000000

#include <stdint.h>
#include "position.h"

/*
	There are special moves for kings and pawns only
	For kings, 1 = O-O, 2 = O-O-O
	For pawns, 4 = Q, 3 = R, 2 = B, 1 = N, 5 = EP
*/

/*

0000 0000 0000 0000 0000 0000 0011 1111  src 0x3f
0000 0000 0000 0000 0000 1111 1100 0000  dst 0xfc0
0000 0000 0000 0000 1111 0000 0000 0000  piece 0xf000
0000 0000 0000 1111 0000 0000 0000 0000  special 0xf0000
0000 0111 1111 0000 0000 0000 0000 0000  epSquare 0x7f00000
0000 1000 0000 0000 0000 0000 0000 0000  is capture 0x8000000
0001 0000 0000 0000 0000 0000 0000 0000  is promotion 0x10000000
0010 0000 0000 0000 0000 0000 0000 0000  is en passant 0x20000000

*/

typedef uint32_t Move;

#define CREATE_MOVE(src, dst, piece, special, eqSquare, capture, promotion, ep) \
	(src) | ((dst) << 6) | ((piece) << 12) | ((special) << 16) | \
	((eqSquare) << 20) | ((capture) << 27) | ((promotion) << 28) | ((ep) << 29)

#define GET_MOVE_SRC(move) ((move) & 0x3f)
#define GET_MOVE_DST(move) (((move) & 0xfc0) >> 6)
#define GET_MOVE_PIECE(move) (((move) & 0xf000) >> 12)
#define GET_MOVE_SPECIAL(move) (((move) & 0xf0000) >> 16)
#define GET_MOVE_EP_SQUARE(move) (((move) & 0x7f00000) >> 20)
#define GET_MOVE_CAPTURE(move) (((move) & 0x8000000) >> 27)
#define GET_MOVE_PROMOTION(move) (((move) & 0x10000000) >> 28)
#define GET_MOVE_EP_PASSANT(move) (((move) & 0x10000000) >> 29)
//#define GET_MOVE_SCORE(move) ((move) >> 32)

int moveEquality(Move m1, Move m2);
int compareMoves(const void * a, const void * b);
GameState playMove(GameState *pos, Move move, int *isLegal);
void printMove(Move m);

#endif