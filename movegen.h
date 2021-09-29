#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "bitboard.h"
#include "move.h"
#include "list.h"

Node *generateMoves(GameState position);
void printMoveList(Node *head, GameState position);

#endif