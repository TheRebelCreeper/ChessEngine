#ifndef MAGIC_H
#define MAGIC_H

typedef unsigned long long U64;

extern int RBits[64];
extern int BBits[64];

U64 random_u64();
U64 random_u64_fewbits();
U64 find_magic_number(int sq, int m, int bishop);

#endif
