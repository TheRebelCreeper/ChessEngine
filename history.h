#ifndef CHESSENGINE_HISTORY_H
#define CHESSENGINE_HISTORY_H

#define HISTORY_SCORE_MAX	4200000
#define HISTORY_SCORE_MIN	600
#include "move.h"
#include "position.h"

extern int history_index; // Used to index 50 move rule array
U64 pos_history[256]; // Used to calculate 50 move rule array

void clear_history();
int score_history(const GameState *pos, Move move, int depth);
void update_history(const GameState *pos, Move move, int bonus);
int get_history(const GameState *pos, Move move);
Move get_killer_one(int ply);
Move get_killer_two(int ply);
void push_killer_move(Move move, int ply);

#endif
