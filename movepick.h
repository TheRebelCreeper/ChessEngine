#ifndef MOVEPICK_H
#define MOVEPICK_H
#include "movelist.h"
#include "search.h"

#define TT_HIT_SCORE		10000000
#define KILLER_ONE			4500000
#define KILLER_TWO			4400000

void pick_move(MoveList *moves, int start_index);
void score_moves(MoveList *moves, const GameState *pos, Move tt_move, int ply);

#endif
