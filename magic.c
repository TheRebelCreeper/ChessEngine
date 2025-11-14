#include "magic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitboard.h"

/* 
Credit to Tord Romstad for original code regarding generating magic numbers
https://www.chessprogramming.org/Looking_for_Magics
I provided some optimization using memset instead of a for loop resetting used.
*/

U64 random_u64()
{
    // define 4 random numbers
    U64 u1, u2, u3, u4;

    // init random numbers slicing 16 bits from MS1B side
    u1 = (U64) (random()) & 0xFFFF;
    u2 = (U64) (random()) & 0xFFFF;
    u3 = (U64) (random()) & 0xFFFF;
    u4 = (U64) (random()) & 0xFFFF;

    // return random number
    return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

U64 random_u64_fewbits()
{
    return random_u64() & random_u64() & random_u64();
}

U64 find_magic_number(int sq, int m, int bishop)
{
    U64 occupancy[4096], a[4096], used[4096];
    int i, j, fail;

    U64 mask = bishop ? calculate_bishop_occupancy(sq) : calculate_rook_occupancy(sq);
    int n = COUNT_BITS(mask);

    for (i = 0; i < (1 << n); i++) {
        occupancy[i] = occupancy_from_index(i, mask);
        a[i] = bishop ? generate_bishop_attacks(sq, occupancy[i]) : generate_rook_attacks(sq, occupancy[i]);
    }
    for (int k = 0; k < 100000000; k++) {
        U64 magic = random_u64_fewbits();
        if (COUNT_BITS((mask * magic) & 0xFF00000000000000ULL) < 6) {
            continue;
        }
        memset(used, 0ULL, sizeof(used));
        for (i = 0, fail = 0; !fail && i < (1 << n); i++) {
            j = (int) ((occupancy[i] * magic) >> (64 - m));
            if (used[j] == 0ULL) {
                used[j] = a[i];
            }
            else if (used[j] != a[i]) {
                fail = 1;
            }
        }
        if (fail == 0) {
            return magic;
        }
    }
    printf("***Failed***\n");
    return 0ULL;
}
