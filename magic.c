#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "magic.h"
#include "bitboard.h"

// Credit to Tord Romstad for generating magic numbers
// https://www.chessprogramming.org/Looking_for_Magics
U64 random_u64()
{
    // define 4 random numbers
    U64 u1, u2, u3, u4;
    
    // init random numbers slicing 16 bits from MS1B side
    u1 = (U64)(random()) & 0xFFFF;
    u2 = (U64)(random()) & 0xFFFF;
    u3 = (U64)(random()) & 0xFFFF;
    u4 = (U64)(random()) & 0xFFFF;
    
    // return random number
    return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

// generate magic number candidate
U64 random_u64_fewbits()
{
    return random_u64() & random_u64() & random_u64();
}