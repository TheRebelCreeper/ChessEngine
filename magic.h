#ifndef MAGIC_H
#define MAGIC_H
#include "bitboard.h"

u64 random_u64();
u64 random_u64_fewbits();
u64 find_magic_number(int sq, int m, int bishop);

#endif
