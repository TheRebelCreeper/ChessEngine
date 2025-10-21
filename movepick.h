#ifndef MOVEPICK_H
#define MOVEPICK_H
#include "movelist.h"
#include "search.h"

#define TT_HIT_SCORE		10000000
#define KILLER_ONE			4500000
#define KILLER_TWO			4400000
#define HISTORY_SCORE_MAX	4200000
#define HISTORY_SCORE_MIN	600

void pick_move(MoveList *moves, int start_index);
void score_moves(MoveList *moves, GameState *pos, Move tt_move, SearchInfo *info);

#endif
