#ifndef ET_H
#define ET_H

#include <stdint.h>
#include "position.h"

#ifndef INF
#define INF 1000000
#endif

extern uint32_t ET_SIZE;

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

extern ET GLOBAL_ET;

void init_et(ET *table);
void clear_et(ET *table);
int probe_et(const GameState *pos);
void save_et(GameState *pos, int eval);

#endif
