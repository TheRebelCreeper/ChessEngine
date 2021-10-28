#ifndef SEARCH_H
#define SEARCH_H

#include "position.h"
#include "move.h"
#include "evaluation.h"

extern int NUM_THREADS;

typedef struct info
{
	int depth;
	unsigned int ms;
	unsigned int nps;
	int bestScore;
	U64 nodes;
} SearchInfo;

Move search(int depth, GameState *pos, SearchInfo *info);

#endif