#ifndef SEARCH_H
#define SEARCH_H

#include "move.h"
#include "position.h"

#define MAX_PLY 128

#define FULL_DEPTH_MOVES 3

#define ASPIRATION_WINDOW

extern int NUM_THREADS;

typedef struct info {
    int starttime;
    int stoptime;
    int depth;
    int timeset;
    int movestogo;

    int stopped;
    unsigned int ms;
    unsigned int nps;
    int best_score;
    int ply;
    U64 nodes;
    Move killer_moves[2][MAX_PLY];
    int pv_table_length[MAX_PLY + 1];
    Move pv_table[MAX_PLY + 1][MAX_PLY + 1];
} SearchInfo;

/*
	PxP PxN PxB PxR PxQ PxK
	NxP NxN NxB NxR NxQ PxK
	BxP BxN BxB BxR BxQ BxK
	RxP RxN RxB RxR RxQ RxK
	QxP QxN QxB QxR QxQ QxK
	KxP KxN KxB KxR KxQ KxK
*/
static const int MVV_LVA_TABLE[6][12] =
{
    {105, 205, 305, 405, 505, 0, 105, 205, 305, 405, 505, 0},
    {104, 204, 304, 404, 504, 0, 104, 204, 304, 404, 504, 0},
    {103, 203, 303, 403, 503, 0, 103, 203, 303, 403, 503, 0},
    {102, 202, 302, 402, 502, 0, 102, 202, 302, 402, 502, 0},
    {101, 201, 301, 401, 501, 0, 101, 201, 301, 401, 501, 0},
    {100, 200, 300, 400, 500, 0, 100, 200, 300, 400, 500, 0}
};

static const int RAZOR_MARGIN[4] = {
    0,
    280,
    300,
    320
};

// Original values were multiplied by 1.5
static const int futility_margins[9] = {0, 150, 240, 330, 420, 510, 600, 690, 780};

int qsearch(int alpha, int beta, GameState *pos, SearchInfo *info);
void search_root(GameState *pos, SearchInfo *root_info);
void read_input(SearchInfo *info);

#endif
