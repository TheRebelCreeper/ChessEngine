#ifndef TT_H
#define TT_H

#include <stdint.h>
#include "evaluation.h"
#include "move.h"

extern uint32_t TT_SIZE;

#define INVALID_SCORE (INF + 100)

#define TT_EXACT   0x4
#define TT_UPPER  0x2
#define TT_LOWER  0x1
#define TT_NONE 0x0

typedef struct ttEntry {
    U64 key;
    unsigned char depth;
    unsigned char flag;
    Move move;
    int score;
} TTEntry;

typedef struct tt {
    TTEntry *hash_table;
    int num_entries;
    int new_write;
    int over_write;
    int hit;
    int cut;
} TT;

extern TT GLOBAL_TT;

void init_tt(TT *table);
void clear_tt(TT *table);
bool probe_tt(const GameState *pos, TTEntry *dst, int ply);
void save_tt(const GameState *pos, Move move, int score, int flag, int depth, int ply);

#endif
