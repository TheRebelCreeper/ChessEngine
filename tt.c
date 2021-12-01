#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "tt.h"


TT GLOBAL_TT;

TTEntry createTTEntry(U64 key, Move move, int score, int depth, char pv, char bound)
{
	TTEntry entry;
	entry.key = key;
	entry.move = move;
	entry.score = score;
	entry.depth = depth;
	entry.bound = pv | bound;
	return entry;
}

void initTT(TT *table)
{
	table->numEntries = TT_SIZE / sizeof(TTEntry);
	table->numEntries -= 2;
	if(table->hashTable != NULL)
		free(table->hashTable);
    table->hashTable = (TTEntry*) malloc(table->numEntries * sizeof(TTEntry));
    clearTT(table);
    printf("HashTable init complete with %d entries\n", table->numEntries);
}

void clearTT(TT *table)
{
	TTEntry *e;
	for (e = table->hashTable; e < table->hashTable + table->numEntries; e++)
	{
		e->key = 0ULL;
		e->move = 0;
		e->depth = 0;
		e->score = 0;
		e->bound = 0;
	}
	table->newWrite=0;
}

// Should return a score
int probeTT(GameState *pos, int *score, Move *move, int alpha, int beta, int depth, int ply)
{
	int hashScore = INVALID_SCORE;
	int index = pos->key % GLOBAL_TT.numEntries;
	TTEntry entry = GLOBAL_TT.hashTable[index];
	
	if(entry.key == pos->key)
	{
		*move = entry.move;
		if(entry.depth >= depth)
		{
			GLOBAL_TT.hit++;
			
			*score = entry.score;
			if(*score > CHECKMATE)
				*score -= ply;
            else if(*score < -CHECKMATE)
				*score += ply;
			
			if (entry.bound == TT_ALL && *score <= alpha)
			{
				*score = alpha;
				return 1;
			}
			else if (entry.bound == TT_CUT && *score >= beta)
			{
				*score = beta;
				return 1;
			}
			else if (entry.bound == TT_PV)
			{
				return 1;
			}
		}
	}
	return 0;
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
	
	if(score > CHECKMATE)
		score += ply;
    else if(score < -CHECKMATE)
		score -= ply;
	
	GLOBAL_TT.hashTable[index].move = move;
    GLOBAL_TT.hashTable[index].key = pos->key;
	GLOBAL_TT.hashTable[index].bound = bound;
	GLOBAL_TT.hashTable[index].score = score;
	GLOBAL_TT.hashTable[index].depth = depth;
}