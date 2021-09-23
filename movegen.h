#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "bitboard.h"
#include "move.h"
#include "list.h"

Node *generateMoves(struct GameState position);
void printMoveList(Node *head, struct GameState position);

#endif