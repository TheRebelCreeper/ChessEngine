#ifndef LIST_H
#define LIST_H
/**
 * list data structure containing the tasks in the system
 */

#include "move.h"
#define MAX_MOVES 256

static const int MVV_LVA_TABLE[6][6] = 
{
	{105, 205, 305, 405, 505, 605},
	{104, 204, 304, 404, 504, 604},
	{103, 203, 303, 403, 503, 603},
	{102, 202, 302, 402, 502, 602},
	{101, 201, 301, 401, 501, 601},
	{100, 200, 300, 400, 500, 600}
};

typedef struct movelist {
	int nextOpen;
	Move list[MAX_MOVES];
} MoveList;

void scoreMoves(MoveList *moves, GameState *pos);
void pickMove(MoveList *moves, int startIndex);

#endif
