#ifndef ET_H
#define ET_H

#include <stdint.h>
#include "position.h"

#ifndef INF
#define INF 1000000
#endif

// 64 Megabytes
#define ET_SIZE ((1 << 20) * 64)

#define INVALID_EVALUATION (INF + 100)

/*#define TT_PV  0x4
#define TT_ALL 0x2
#define TT_CUT 0x1

#define TT_BOUND(entry) ((entry) & 0x3)
#define TT_IS_PV(entry) ((entry) & TT_PV)
#define TT_GENERATION(entry) ((entry) & F8)*/

typedef struct etEntry {
	U64 key;
	int eval;
} ETEntry;

typedef struct et{
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
