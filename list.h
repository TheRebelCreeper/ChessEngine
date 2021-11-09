#ifndef LIST_H
#define LIST_H
/**
 * list data structure containing the tasks in the system
 */

#include "move.h"
#define MAX_MOVES 256

typedef struct movelist {
	int nextOpen;
	Move list[MAX_MOVES];
} MoveList;

void printMoveList(MoveList *list);

#endif
