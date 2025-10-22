#include "history.h"

#include <assert.h>
#include <string.h>

#include "move.h"
#include "util.h"

int history_index = 0;

inline void clear_history()
{
    memset(history_table, 0, sizeof(history_table));
}

inline int score_history(const GameState *pos, Move move, int depth)
{
    assert(move != 0 && depth >= 0);
    int bonus = history_table[pos->turn][GET_MOVE_SRC(move)][GET_MOVE_DST(move)];
    return MIN(bonus + (depth * depth), HISTORY_SCORE_MAX);
}

inline void update_history(const GameState *pos, Move move, int bonus)
{
    assert(move != 0);
    history_table[pos->turn][GET_MOVE_SRC(move)][GET_MOVE_DST(move)] = bonus;
}

inline int get_history(const GameState *pos, Move move)
{
    assert(move != 0);
    return history_table[pos->turn][GET_MOVE_SRC(move)][GET_MOVE_DST(move)];
}
