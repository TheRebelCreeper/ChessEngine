#ifndef SEE_H
#define SEE_H

#include <stdbool.h>

#include "move.h"
#include "position.h"

bool see_at_least(const GameState *pos, Move move, int threshold);

#endif
