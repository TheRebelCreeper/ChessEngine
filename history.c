#include "history.h"

#include <assert.h>
#include <string.h>

#include "move.h"
#include "util.h"

Move killer_table[2][MAX_PLY];
int history_table[2][64][64];
int repetition_index = 0;

inline void clear_history()
{
    memset(history_table, 0, sizeof(history_table));
}

inline int score_history(const GameState *pos, Move move, int depth)
{
    assert(move != 0 && depth >= 0);
    int bonus = depth * depth;
    return bonus;
}

inline void update_history(const GameState *pos, Move move, int bonus)
{
    assert(move != 0);
    int tmp = history_table[pos->turn][GET_MOVE_SRC(move)][GET_MOVE_DST(move)];
    history_table[pos->turn][GET_MOVE_SRC(move)][GET_MOVE_DST(move)] = MIN(tmp + bonus, HISTORY_SCORE_MAX);
}

inline int get_history(const GameState *pos, Move move)
{
    assert(move != 0);
    return history_table[pos->turn][GET_MOVE_SRC(move)][GET_MOVE_DST(move)];
}

inline Move get_killer_one(int ply)
{
    return killer_table[0][ply];
}

inline Move get_killer_two(int ply)
{
    return killer_table[0][ply];
}

inline void push_killer_move(Move move, int ply)
{
    Move killer_one = killer_table[0][ply];
    if (move != killer_one) {
        killer_table[1][ply] = killer_one;
        killer_table[0][ply] = move;
    }
}
