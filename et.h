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
    U64 key;
    int eval;
} ETEntry;

typedef struct et {
    ETEntry *hashTable;
    int numEntries;
    int newWrite;
    int overWrite;
    int hit;
    int cut;
} ET;

extern ET GLOBAL_ET;

void initET(ET *table);
void clearET(ET *table);
int probeET(GameState *pos);
void saveET(GameState *pos, int eval);

#endif
