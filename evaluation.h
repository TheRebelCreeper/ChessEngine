#ifndef EVALUATION_H
#define EVALUATION_H

#include "position.h"

extern int INF;
extern int MATE_SCORE;
extern int MAX_MATE_SCORE;

static const int piece_value[12] = {100, 310, 330, 500, 900, 10000, -100, -310, -330, -500, -900, -10000};

static const int nnue_pieces[12] =
{
    6, 5, 4, 3, 2, 1, 12, 11, 10, 9, 8, 7
};

void print_evaluation(int score);
int nnue_eval(const GameState *pos);
int evaluation(GameState *pos);
int see(const GameState *pos, int square);

#endif
