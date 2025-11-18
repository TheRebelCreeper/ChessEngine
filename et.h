#ifndef ET_H
#define ET_H

#include "position.h"

#ifndef INF
#define INF 1000000
#endif

#define INVALID_EVALUATION (INF + 100)

typedef struct etEntry {
    int key;
    int eval;
} ETEntry;

typedef struct et {
    ETEntry *hash_table;
    int num_entries;
    int new_write;
    int over_write;
    int hit;
    int cut;
} ET;

void init_et();
void clear_et();
void set_et_size(int mb);
int probe_et(const GameState *pos);
void save_et(GameState *pos, int eval);

#endif
