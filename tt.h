#ifndef TT_H
#define TT_H

#include <stdint.h>
#include "move.h"

// 16 Megabytes
#define TT_SIZE ((1 << 20) * 32)

#define TT_PV  0x4
#define TT_ALL 0x2
#define TT_CUT 0x1

#define TT_BOUND(entry) ((entry) & 0x3)
#define TT_IS_PV(entry) ((entry) & TT_PV)
#define TT_GENERATION(entry) ((entry) & F8)

typedef struct ttEntry {
	U64 key;
	int depth;
	unsigned char bound;
	Move move;
	int score;
} TTEntry;

typedef struct tt{
	TTEntry *hashTable;
	int numEntries;
	int newWrite;
	int overWrite;
	int hit;
	int cut;
} TT;

extern TT GLOBAL_TT;

TTEntry createTTEntry(U64 key, Move move, int score, int depth, char pv, char bound);
void initTT(TT *table);
void clearTT(TT *table);
int probeTT(GameState *pos, int *score, Move *move, int alpha, int beta, int depth, int ply);
void saveTT(GameState *pos, Move move, int score, int bounds, int depth, int ply);

#endif