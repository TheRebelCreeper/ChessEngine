#ifndef MAGIC_H
#define MAGIC_H
#include "bitboard.h"

U64 random_u64();
U64 random_u64_fewbits();
U64 find_magic_number(int sq, int m, int bishop);

#endif
