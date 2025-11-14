#include "movelist.h"
#include <string.h>

void clear_movelist(MoveList *l)
{
    l->next_open = 0;
    memset(l->move, 0, MAX_MOVES * sizeof(Move));
}
