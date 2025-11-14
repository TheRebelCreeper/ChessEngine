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

#define SET_SQUARE(board, square) ((board) |= (1ULL << (square)))
#define CLEAR_SQUARE(board, square) ((board) &= ~(1ULL << (square)))
#define CLEAR_LSB(board) ((board) = ((board) & ((board) - 1)))
#define GET_SQUARE(board, square) ((board) & (1ULL << (square)))

#define FILE_A 0x0101010101010101ULL
#define FILE_B (FILE_A << 1)
#define FILE_C (FILE_A << 2)
#define FILE_D (FILE_A << 3)
#define FILE_E (FILE_A << 4)
#define FILE_F (FILE_A << 5)
#define FILE_G (FILE_A << 6)
#define FILE_H (FILE_A << 7)

#define RANK_1 0xFFULL
#define RANK_2 (RANK_1 << (8 * 1))
#define RANK_3 (RANK_1 << (8 * 2))
#define RANK_4 (RANK_1 << (8 * 3))
#define RANK_5 (RANK_1 << (8 * 4))
#define RANK_6 (RANK_1 << (8 * 5))
#define RANK_7 (RANK_1 << (8 * 6))
#define RANK_8 (RANK_1 << (8 * 7))

extern char *square_names[65];

U64 king_attacks[64];
U64 knight_attacks[64];
U64 pawn_attacks[2][64];

U64 bishop_occupancy[64];
U64 rook_occupancy[64];

U64 bishop_magic[64];
U64 rook_magic[64];

U64 bishop_attacks[64][512];
U64 rook_attacks[64][4096];

U64 generate_bishop_attacks(int square, U64 blockers);
U64 generate_rook_attacks(int square, U64 blockers);
U64 occupancy_from_index(int index, U64 board);
U64 calculate_bishop_occupancy(int square);
U64 calculate_rook_occupancy(int square);
U64 get_bishop_attacks(int square, U64 blockers);
U64 get_rook_attacks(int square, U64 blockers);
U64 get_queen_attacks(int square, U64 blockers);

#define COUNT_BITS(board) (__builtin_popcountll((board)))
#define GET_FIRST_BIT_SQUARE(board) (__builtin_ffsll((board)) - 1)

void init_leapers();
void init_sliders();
void init_attacks();
void print_bitboard(U64 board);

#endif
