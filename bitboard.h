#ifndef BITBOARD_H
#define BITBOARD_H

#define DEBUG

typedef unsigned long long U64;

enum
{
	a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8,
};

#define set_square(board, square) ((board) |= (1ULL << (square)))
#define clear_square(board, square) ((board) &= ~(1ULL << (square)))
#define get_square(board, square) ((board) & (1ULL << (square)))

extern const U64 FileA;
extern const U64 FileB;
extern const U64 FileC;
extern const U64 FileD;
extern const U64 FileE;
extern const U64 FileF;
extern const U64 FileG;
extern const U64 FileH;

extern const U64 Rank1;
extern const U64 Rank2;
extern const U64 Rank3;
extern const U64 Rank4;
extern const U64 Rank5;
extern const U64 Rank6;
extern const U64 Rank7;
extern const U64 Rank8;

U64 KingAttacks[64];
U64 KnightAttacks[64];
U64 PawnAttacks[2][64];

U64 BishopOccupancy[64];
U64 RookOccupancy[64];

U64 BishopAttacks[64][512];
U64 RookAttacks[64][4096];

U64 generateBishopAttacks(int rank, int file, U64 blockers);
U64 generateRookAttacks(int rank, int file, U64 blockers);
U64 occupancyFromIndex(int index, U64 board);

int countBits(U64 board);

void initLeapers();
void initSliders();
void printBitboard(U64 board);

#endif