#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "et.h"

// Default to 16 MB
uint32_t ET_SIZE = ((1 << 20) * 16);
ET GLOBAL_ET;

void initET(ET *table)
{
	table->numEntries = ET_SIZE / sizeof(ETEntry);
	table->numEntries -= 2;
	if (table->hashTable != NULL)
		free(table->hashTable);
	table->hashTable = (ETEntry *) malloc(table->numEntries * sizeof(ETEntry));
	if (table->hashTable == NULL) {
		perror("Failed to allocate hash table\n");
	}
	clearET(table);
	printf("EvalTable init complete with %d entries\n", table->numEntries);
}

void clearET(ET *table)
{
	ETEntry *e;
	for (e = table->hashTable; e < table->hashTable + table->numEntries; e++) {
		e->key = 0ULL;
		e->eval = INVALID_EVALUATION;
	}
	table->newWrite = 0;
}

// Should return a score
int probeET(GameState *pos)
{
	int index = pos->key % GLOBAL_ET.numEntries;
	ETEntry entry = GLOBAL_ET.hashTable[index];

	if (entry.key == pos->key && entry.eval != INVALID_EVALUATION) {
		return entry.eval;
	}
	return INVALID_EVALUATION;
}

void saveET(GameState *pos, int eval)
{
	int index = pos->key % GLOBAL_ET.numEntries;

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

	GLOBAL_ET.hashTable[index].key = pos->key;
	GLOBAL_ET.hashTable[index].eval = eval;
}
