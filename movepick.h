#ifndef MOVEPICK_H
#define MOVEPICK_H
#include "movelist.h"
#include "search.h"

void pick_move(MoveList *moves, int start_index);
void score_moves(MoveList *moves, GameState *pos, Move tt_move, SearchInfo *info);

#endif
