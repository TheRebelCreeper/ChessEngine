#ifndef TT_H
#define TT_H

#include "evaluation.h"
#include "move.h"

#define TT_EXACT   0x4
#define TT_UPPER  0x2
#define TT_LOWER  0x1
#define TT_NONE 0x0

typedef struct ttEntry {
    int key;
    unsigned char depth;
    unsigned char flag;
    Move move;
    int score;
    int static_eval;
} TTEntry;

typedef struct tt {
    TTEntry *hash_table;
    int num_entries;
    int new_write;
    int over_write;
    int hit;
    int cut;
} TT;

void init_tt();
void clear_tt();
void set_tt_size(int mb);
bool probe_tt(const GameState *pos, TTEntry *dst, int ply);
void save_tt(const GameState *pos, Move move, int score, int static_eval, int flag, int depth, int ply);

#endif
