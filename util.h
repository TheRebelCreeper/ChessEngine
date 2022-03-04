#ifndef UTIL_H
#define UTIL_H

#include "search.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

int GetTimeMs();
int InputWaiting();
void ReadInput(SearchInfo *info);

#endif