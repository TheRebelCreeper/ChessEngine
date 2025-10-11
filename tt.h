#ifndef TT_H
#define TT_H

#include <stdint.h>
#include "evaluation.h"
#include "move.h"

extern uint32_t TT_SIZE;

#define INVALID_SCORE (INF + 100)

#define TT_PV  0x4
#define TT_ALL 0x2
#define TT_CUT 0x1

typedef struct ttEntry {
    U64 key;
    unsigned char depth;
    unsigned char bound;
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
int probe_tt(const GameState *pos, Move *move, int alpha, int beta, int depth, int ply);
void save_tt(const GameState *pos, Move move, int score, int bounds, int depth, int ply);

#endif
