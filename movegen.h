#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "movelist.h"

int generate_moves_qsearch(const GameState *position, MoveList *move_list);
int generate_moves(const GameState *position, MoveList *move_list);

#endif
