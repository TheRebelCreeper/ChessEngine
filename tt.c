#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "tt.h"
#include "evaluation.h"

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

int probeTT(GameState *pos, int *score, int alpha, int beta, int depth, int ply)
{
	int index = pos->key % GLOBAL_TT.numEntries;
	assert(index >= 0 && index <= GLOBAL_TT.numEntries - 1);
    assert(depth>=1&&depth<64);
    assert(alpha<beta);
    assert(alpha>=-CHECKMATE&&alpha<=CHECKMATE);
    assert(beta>=-CHECKMATE&&beta<=CHECKMATE);
    assert(ply>=0&&ply<64);
	
	TTEntry entry = GLOBAL_TT.hashTable[index];
	
	if(entry.key == pos->key)
	{
		if(entry.depth >= depth){
			GLOBAL_TT.hit++;
			
			assert(entry.depth>=1&&entry.depth<64);
            assert(entry.bound>=TT_CUT&&entry.bound<=TT_PV);
			
			*score = entry.score;
			if(*score > MAX_PLY_CHECKMATE) *score -= ply;
            else if(*score < -MAX_PLY_CHECKMATE) *score += ply;
			
			switch(entry.bound) {
                case TT_ALL: if(*score<=alpha) {
                    *score=alpha;
                    return 1;
                    }
                    break;
                case TT_CUT: if(*score>=beta) {
                    *score=beta;
                    return 1;
                    }
                    break;
                case TT_PV:
                    return 1;
                    break;
                default: assert(0); break;
            }
		}
	}
	return 0;
}

void saveTT(GameState *pos, int move, int score, int bound, int depth, int ply)
{

	int index = pos->key % GLOBAL_TT.numEntries;
	
	assert(index >= 0 && index <= GLOBAL_TT.numEntries - 1);
	assert(depth>=1&&depth<64);
    assert(score>=-CHECKMATE&&score<=CHECKMATE);
    assert(ply>=0&&ply<64);
	
	if( GLOBAL_TT.hashTable[index].key == 0) {
		GLOBAL_TT.newWrite++;
	} else {
		GLOBAL_TT.overWrite++;
	}
	
	if(score > MAX_PLY_CHECKMATE) score += ply;
    else if(score < -MAX_PLY_CHECKMATE) score -= ply;
	
	GLOBAL_TT.hashTable[index].move = move;
    GLOBAL_TT.hashTable[index].key = pos->key;
	GLOBAL_TT.hashTable[index].bound = bound;
	GLOBAL_TT.hashTable[index].score = score;
	GLOBAL_TT.hashTable[index].depth = depth;
}