#ifndef BITBOARD_H
#define BITBOARD_H

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

extern const U64 AFile;
extern const U64 BFile;
extern const U64 CFile;
extern const U64 DFile;
extern const U64 EFile;
extern const U64 FFile;
extern const U64 GFile;
extern const U64 HFile;

void print_bitboard(U64 board);

#endif