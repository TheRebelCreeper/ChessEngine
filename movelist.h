#ifndef LIST_H
#define LIST_H

#include "move.h"
#define MAX_MOVES 256

typedef struct movelist {
    int next_open;
    Move move[MAX_MOVES];
    int score[MAX_MOVES];
} MoveList;

void clear_movelist(MoveList *l);

#endif
