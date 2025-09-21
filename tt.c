#include "tt.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// Default memory to 128 MB
uint32_t TT_SIZE = ((1 << 20) * 128);
TT GLOBAL_TT;

void initTT(TT *table)
{
	table->numEntries = TT_SIZE / sizeof(TTEntry);
	table->numEntries -= 2;
	if (table->hashTable != NULL)
		free(table->hashTable);
	table->hashTable = (TTEntry *) malloc(table->numEntries * sizeof(TTEntry));
	if (table->hashTable == NULL) {
		perror("Failed to allocate hash table\n");
	}
	clearTT(table);
	printf("HashTable init complete with %d entries\n", table->numEntries);
}

void clearTT(TT *table)
{
	TTEntry *e;
	for (e = table->hashTable; e < table->hashTable + table->numEntries; e++) {
		e->key = 0ULL;
		e->move = 0;
		e->depth = 0;
		e->score = INVALID_SCORE;
		e->bound = 0;
	}
	table->newWrite = 0;
}

// Should return a score
int probeTT(GameState *pos, Move *move, int alpha, int beta, int depth, int ply)
{
	int finalScore = INVALID_SCORE;
	int index = pos->key % GLOBAL_TT.numEntries;
	TTEntry entry = GLOBAL_TT.hashTable[index];

	if (entry.key == pos->key) {
		*move = entry.move;
		if (entry.depth >= depth) {
			GLOBAL_TT.hit++;

			int score = entry.score;
			if (score > CHECKMATE)
				score -= ply;
			else if (score < -CHECKMATE)
				score += ply;

			if (entry.bound == TT_ALL && score <= alpha) {
				finalScore = alpha;
			}
			else if (entry.bound == TT_CUT && score >= beta) {
				finalScore = beta;
			}
			else if (entry.bound == TT_PV) {
				finalScore = score;
			}
		}
	}
	return finalScore;
}

void saveTT(GameState *pos, Move move, int score, int bound, int depth, int ply)
{
	int index = pos->key % GLOBAL_TT.numEntries;

	/*
	// Debug stats
	if( GLOBAL_TT.hashTable[index].key == 0)
	{
		GLOBAL_TT.newWrite++;
	}
	else
	{
		GLOBAL_TT.overWrite++;
	}*/

	if (score > CHECKMATE)
		score += ply;
	else if (score < -CHECKMATE)
		score -= ply;

	GLOBAL_TT.hashTable[index].move = move;
	GLOBAL_TT.hashTable[index].key = pos->key;
	GLOBAL_TT.hashTable[index].bound = bound;
	GLOBAL_TT.hashTable[index].score = score;
	GLOBAL_TT.hashTable[index].depth = (unsigned char) depth;
}
