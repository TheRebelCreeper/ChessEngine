#ifndef MOVE_H
#define MOVE_H

#define OO_SPECIAL 1
#define OOO_SPECIAL 2
#define EN_PASSANT_SPECIAL 5
#define NO_SPECIAL 0

#include "position.h"

int compareMoves(Move m1, Move m2);
Move createMove(int piece, int src, int dst, int special, int epSquare);
GameState playMove(GameState pos, Move move);

#endif