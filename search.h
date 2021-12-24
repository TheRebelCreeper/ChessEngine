#ifndef SEARCH_H
#define SEARCH_H

#include "position.h"
#include "move.h"
#include "evaluation.h"

#define MAX_PLY 64

#define HISTORY_SCORE_MAX 4200
#define KILLER_ONE 4500
#define KILLER_TWO 4400
#define TT_HIT_SCORE 10000

#define FULL_DEPTH_MOVES 2
#define REDUCTION_LIMIT 3

#define ASPIRATION_WINDOW

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

extern int NUM_THREADS;

typedef struct info
{
	int starttime;
	int stoptime;
	int depth;
	int timeset;
	int movestogo;

	int stopped;
	unsigned int ms;
	unsigned int nps;
	int bestScore;
	int ply;
	U64 nodes;
	Move killerMoves[2][MAX_PLY];
	int history[2][64][64];
	int pvTableLength[MAX_PLY];
	Move pvTable[MAX_PLY][MAX_PLY];
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

static const int RAZOR_MARGIN[4] = {0,
    280,
    300,
    320
};

// Original values were multiplied by 1.5
static const int futilityMargins[9] = {0, 150, 240, 330, 420, 510, 600, 690, 780};

int quiescence(int alpha, int beta, int depth, GameState *pos, SearchInfo *info);
void search(GameState *pos, SearchInfo *info);
int GetTimeMs();
void ReadInput(SearchInfo *info);

#endif