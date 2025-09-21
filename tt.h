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

#define TT_BOUND(entry) ((entry) & 0x3)
#define TT_IS_PV(entry) ((entry) & TT_PV)
#define TT_GENERATION(entry) ((entry) & F8)

typedef struct ttEntry {
	U64 key;
	unsigned char depth;
	unsigned char bound;
	Move move;
	int score;
} TTEntry;

typedef struct tt {
	TTEntry *hashTable;
	int numEntries;
	int newWrite;
	int overWrite;
	int hit;
	int cut;
} TT;

extern TT GLOBAL_TT;

void initTT(TT *table);
void clearTT(TT *table);
int probeTT(GameState *pos, Move *move, int alpha, int beta, int depth, int ply);
void saveTT(GameState *pos, Move move, int score, int bounds, int depth, int ply);

#endif
