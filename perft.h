#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "movegen.h"

#ifndef PERFT_H
#define PERFT_H

void runPerft(int depth, GameState *pos);

#endif