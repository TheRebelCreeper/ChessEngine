#ifndef LIST_H
#define LIST_H
/**
 * list data structure containing the tasks in the system
 */

#include "move.h"
#define MAX_MOVES 256

typedef struct movelist {
    int next_open;
    Move list[MAX_MOVES];
    int score[MAX_MOVES];
} MoveList;

#endif
