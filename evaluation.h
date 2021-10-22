#ifndef EVALUATION_H
#define EVALUATION_H

#include "position.h"

#define CHECKMATE 1000000
#define MAX_PLY_CHECKMATE 999900

void printEvaluation(int score);
int evaluation(GameState *pos);

#endif