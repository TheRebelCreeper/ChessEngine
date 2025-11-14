#ifndef UTIL_H
#define UTIL_H

#include "search.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

int get_time_ms();
int InputWaiting();
void read_input(SearchInfo *info);

#endif
