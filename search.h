#ifndef SEARCH_H
#define SEARCH_H

#include "position.h"
#include "move.h"
#include "evaluation.h"

#define MAX_PLY 64

#define HISTORY_SCORE_MAX 4200000
#define KILLER_ONE 4500000
#define KILLER_TWO 4400000
#define TT_HIT_SCORE 10000000

#define FULL_DEPTH_MOVES 3
#define REDUCTION_LIMIT 3

#define ASPIRATION_WINDOW

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

/*
	PxP PxN PxB PxR PxQ PxK
	NxP NxN NxB NxR NxQ PxK
	BxP BxN BxB BxR BxQ BxK
	RxP RxN RxB RxR RxQ RxK
	QxP QxN QxB QxR QxQ QxK
	KxP KxN KxB KxR KxQ KxK
*/
static const int MVV_LVA_TABLE[6][6] = 
{
	{105, 205, 305, 405, 505, 0},
	{104, 204, 304, 404, 504, 0},
	{103, 203, 303, 403, 503, 0},
	{102, 202, 302, 402, 502, 0},
	{101, 201, 301, 401, 501, 0},
	{100, 200, 300, 400, 500, 0}
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