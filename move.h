#ifndef MOVE_H
#define MOVE_H

#define OO_SPECIAL 1
#define OOO_SPECIAL 2
#define EN_PASSANT_SPECIAL 5
#define NO_SPECIAL 0

#define IS_CAPTURE 0xf00000
#define IS_PAWN_PUSH 0x1000000
#define IS_EN_PASSANT 0x2000000
#define IS_CASTLES 0x4000000

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
0000 0000 0000 1111 0000 0000 0000 0000  promotion 0xf0000
0000 0000 1111 0000 0000 0000 0000 0000  captured 0xf00000
0000 0001 0000 0000 0000 0000 0000 0000  doublepawnpush 0x1000000
0000 0010 0000 0000 0000 0000 0000 0000  is en passant 0x2000000
0000 0100 0000 0000 0000 0000 0000 0000  is castles 0x4000000

*/

typedef uint32_t Move;

#define CREATE_MOVE(src, dst, piece, promotion, captured, dpp, ep, castles) \
	(src) | ((dst) << 6) | ((piece) << 12) | ((promotion) << 16) | \
	((captured) << 20) | ((dpp) << 24) | ((ep) << 25) | ((castles) << 26)

#define GET_MOVE_SRC(move) ((move) & 0x3f)
#define GET_MOVE_DST(move) (((move) & 0xfc0) >> 6)
#define GET_MOVE_PIECE(move) (((move) & 0xf000) >> 12)
#define GET_MOVE_PROMOTION(move) (((move) & 0xf0000) >> 16)
#define GET_MOVE_CAPTURED(move) (((move) & 0xf00000) >> 20)
#define IS_MOVE_DPP(move) (((move) & 0x1000000) >> 24)
#define IS_MOVE_EP(move) (((move) & 0x2000000) >> 25)
#define IS_MOVE_CASTLES(move) (((move) & 0x4000000) >> 26)

GameState playMove(GameState *pos, Move move, int *isLegal);
void printMove(Move m);

#endif