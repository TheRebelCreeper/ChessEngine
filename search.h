#ifndef SEARCH_H
#define SEARCH_H

#include "position.h"
#include "move.h"
#include "evaluation.h"

#define MAX_PLY 16

extern int NUM_THREADS;

typedef struct info
{
	int depth;
	unsigned int ms;
	unsigned int nps;
	int bestScore;
	U64 nodes;
	Move killerMoves[2][MAX_PLY];
	int history[2][64][64];
} SearchInfo;

static const int MVV_LVA_TABLE[6][6] = 
{
	{105, 205, 305, 405, 505, 605},
	{104, 204, 304, 404, 504, 604},
	{103, 203, 303, 403, 503, 603},
	{102, 202, 302, 402, 502, 602},
	{101, 201, 301, 401, 501, 601},
	{100, 200, 300, 400, 500, 600}
};

Move search(int depth, GameState *pos, SearchInfo *info);

#endif