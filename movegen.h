#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "bitboard.h"
#include "move.h"
#include "list.h"

MoveList generateMoves(GameState position, int *size);
void printMoveList(Node *head, GameState position);

#endif