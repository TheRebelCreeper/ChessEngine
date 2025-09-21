#ifndef BITBOARD_H
#define BITBOARD_H

#define WHITE 0
#define BLACK 1

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
(byte & 0x80 ? '1' : '0'), \
(byte & 0x40 ? '1' : '0'), \
(byte & 0x20 ? '1' : '0'), \
(byte & 0x10 ? '1' : '0'), \
(byte & 0x08 ? '1' : '0'), \
(byte & 0x04 ? '1' : '0'), \
(byte & 0x02 ? '1' : '0'), \
(byte & 0x01 ? '1' : '0')

typedef unsigned long long U64;

enum {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8, none
};

#define set_square(board, square) ((board) |= (1ULL << (square)))
#define clear_square(board, square) ((board) &= ~(1ULL << (square)))
#define clear_lsb(board) ((board) = ((board) & ((board) - 1)))
#define get_square(board, square) ((board) & (1ULL << (square)))

#define FileA 0x0101010101010101ULL
#define FileB (FileA << 1)
#define FileC (FileA << 2)
#define FileD (FileA << 3)
#define FileE (FileA << 4)
#define FileF (FileA << 5)
#define FileG (FileA << 6)
#define FileH (FileA << 7)

#define Rank1 0xFFULL
#define Rank2 (Rank1 << (8 * 1))
#define Rank3 (Rank1 << (8 * 2))
#define Rank4 (Rank1 << (8 * 3))
#define Rank5 (Rank1 << (8 * 4))
#define Rank6 (Rank1 << (8 * 5))
#define Rank7 (Rank1 << (8 * 6))
#define Rank8 (Rank1 << (8 * 7))

extern char *squareNames[65];

U64 kingAttacks[64];
U64 knightAttacks[64];
U64 pawnAttacks[2][64];

U64 bishopOccupancy[64];
U64 rookOccupancy[64];

U64 bishopMagic[64];
U64 rookMagic[64];

U64 bishopAttacks[64][512];
U64 rookAttacks[64][4096];

U64 generateBishopAttacks(int square, U64 blockers);
U64 generateRookAttacks(int square, U64 blockers);
U64 occupancyFromIndex(int index, U64 board);
U64 calculateBishopOccupancy(int square);
U64 calculateRookOccupancy(int square);
U64 getBishopAttacks(int square, U64 blockers);
U64 getRookAttacks(int square, U64 blockers);
U64 getQueenAttacks(int square, U64 blockers);

#define countBits(board) (__builtin_popcountll((board)))
#define getFirstBitSquare(board) (__builtin_ffsll((board)) - 1)

void initLeapers();
void initSliders();
void initAttacks();
void printBitboard(U64 board);

#endif
