#ifndef SEARCH_H
#define SEARCH_H

#include "position.h"
#include "move.h"
#include "evaluation.h"

extern int NUM_THREADS;

Move search(int depth, GameState *pos, int *score);
//int negaMax(int depth, GameState *pos);

#endif