#ifndef SEARCH_H
#define SEARCH_H

#include <stdbool.h>

#include "move.h"
#include "movelist.h"
#include "position.h"

#define MAX_PLY 128

#define MIN_ASP_DEPTH 5
#define INITIAL_ASP_WINDOW 30
#define MIN_LMR_MOVES 3
#define MIN_LMR_DEPTH 2
#define MIN_IIR_DEPTH 3

extern int NUM_THREADS;

typedef struct info {
    int starttime;
    int stoptime;
    int depth;
    bool timeset;
    int movestogo;

    bool stopped;
    bool benchmark;
    unsigned int ms;
    unsigned int nps;
    int ply;
    U64 nodes;
    int pv_table_length[MAX_PLY + 1];
    Move pv_table[MAX_PLY + 1][MAX_PLY + 1];
    Move move_stack[MAX_PLY];
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

void init_lmr_table();
int qsearch(int alpha, int beta, GameState *pos, SearchInfo *info);
void search_root(GameState *pos, SearchInfo *search_info);
void read_input(SearchInfo *info);

#endif
