#ifndef SEARCH_H
#define SEARCH_H

#include "position.h"
#include "move.h"

Move search(int depth, GameState pos, int *score);
int negaMax(int depth, GameState pos);

#endif