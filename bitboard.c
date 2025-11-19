#include "bitboard.h"
#include <stdio.h>
#include "magic.h"

static const int rook_bits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12
};

static const int bishop_bits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6
};

const char *square_names[65] = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8", "-"
};

U64 occupancy_from_index(int index, U64 board)
{
    U64 occupancy = 0ULL;
    int bits = COUNT_BITS(board);

    // Cannot be easily parallelized since order matters regarding 1st least significant bit
    for (int i = 0; i < bits; i++) {
        int square = GET_FIRST_BIT_SQUARE(board);
        CLEAR_SQUARE(board, square);

        // Check if the ith bit is marked in the index
        if (index & (1 << i)) {
            // Add the square to occupancy
            SET_SQUARE(occupancy, square);
        }
    }
    return occupancy;
}

U64 calculate_knight_attacks(int square)
{
    U64 piece_location = 0ULL;
    U64 attacks = 0ULL;
    SET_SQUARE(piece_location, square);

    attacks |= (piece_location << 17 & ~FILE_A); // Up 2 right 1
    attacks |= (piece_location << 15 & ~FILE_H); // Up 2 left 1
    attacks |= (piece_location << 10 & ~(FILE_A | FILE_B)); // Up 1 right 2
    attacks |= (piece_location << 6 & ~(FILE_H | FILE_G)); // Up 1 left 2

    attacks |= (piece_location >> 17 & ~FILE_H); // Down 2 left 1
    attacks |= (piece_location >> 15 & ~FILE_A); // Down 2 right 1
    attacks |= (piece_location >> 10 & ~(FILE_H | FILE_G)); // Down 1 left 2
    attacks |= (piece_location >> 6 & ~(FILE_A | FILE_B)); // Down 1 right 2
    return attacks;
}

U64 calculate_king_attacks(int square)
{
    U64 piece_location = 0ULL;
    U64 attacks = 0ULL;
    SET_SQUARE(piece_location, square);

    attacks |= (piece_location << 1 & ~FILE_A); // Left
    attacks |= (piece_location << 7 & ~FILE_H); // Top right
    attacks |= (piece_location << 8); // Top
    attacks |= (piece_location << 9 & ~FILE_A); // Top left

    attacks |= (piece_location >> 1 & ~FILE_H); // Right
    attacks |= (piece_location >> 7 & ~FILE_A); // Bottom left
    attacks |= (piece_location >> 8); // Bottom
    attacks |= (piece_location >> 9 & ~FILE_H); // Bottom right

    return attacks;
}

U64 calculate_pawn_attacks(int side, int square)
{
    U64 piece_location = 0ULL;
    U64 attacks = 0ULL;
    SET_SQUARE(piece_location, square);

    // White attacks
    if (side == WHITE) {
        attacks |= (piece_location << 9 & ~FILE_A); // Top left
        attacks |= (piece_location << 7 & ~FILE_H); // Top right
    }
    else {
        attacks |= (piece_location >> 9 & ~FILE_H); // Bottom right
        attacks |= (piece_location >> 7 & ~FILE_A); // Bottom left
    }
    return attacks;
}

U64 calculate_bishop_occupancy(int square)
{
    U64 occupancy = 0ULL;
    int s;

    int rank = square / 8;
    int file = square % 8;

    // Calculate bottom left
    int rank_tmp = rank - 1;
    int file_tmp = file - 1;
    while (rank_tmp > 0 && file_tmp > 0) {
        s = rank_tmp * 8 + file_tmp;
        SET_SQUARE(occupancy, s);
        rank_tmp--;
        file_tmp--;
    }

    // Calculate top left
    rank_tmp = rank + 1;
    file_tmp = file - 1;
    while (rank_tmp < 7 && file_tmp > 0) {
        s = rank_tmp * 8 + file_tmp;
        SET_SQUARE(occupancy, s);
        rank_tmp++;
        file_tmp--;
    }

    // Calculate top right
    rank_tmp = rank + 1;
    file_tmp = file + 1;
    while (rank_tmp < 7 && file_tmp < 7) {
        s = rank_tmp * 8 + file_tmp;
        SET_SQUARE(occupancy, s);
        rank_tmp++;
        file_tmp++;
    }

    // Calculate top right
    rank_tmp = rank - 1;
    file_tmp = file + 1;
    while (rank_tmp > 0 && file_tmp < 7) {
        s = rank_tmp * 8 + file_tmp;
        SET_SQUARE(occupancy, s);
        rank_tmp--;
        file_tmp++;
    }
    return occupancy;
}

U64 calculate_rook_occupancy(int square)
{
    U64 occupancy = 0ULL;
    int s;

    int rank = square / 8;
    int file = square % 8;

    // Calculate left
    int rank_tmp = rank;
    int file_tmp = file - 1;
    while (file_tmp > 0) {
        s = rank_tmp * 8 + file_tmp;
        SET_SQUARE(occupancy, s);
        file_tmp--;
    }

    // Calculate right
    rank_tmp = rank;
    file_tmp = file + 1;
    while (file_tmp < 7) {
        s = rank_tmp * 8 + file_tmp;
        SET_SQUARE(occupancy, s);
        file_tmp++;
    }

    // Calculate top
    rank_tmp = rank + 1;
    file_tmp = file;
    while (rank_tmp < 7) {
        s = rank_tmp * 8 + file_tmp;
        SET_SQUARE(occupancy, s);
        rank_tmp++;
    }

    // Calculate bottom
    rank_tmp = rank - 1;
    file_tmp = file;
    while (rank_tmp > 0) {
        s = rank_tmp * 8 + file_tmp;
        SET_SQUARE(occupancy, s);
        rank_tmp--;
    }
    return occupancy;
}

U64 generate_bishop_attacks(int square, U64 blockers)
{
    U64 occupancy = 0ULL;
    int s;

    int rank = square / 8;
    int file = square % 8;

    // Calculate bottom left
    int rank_tmp = rank - 1;
    int file_tmp = file - 1;
    while (rank_tmp >= 0 && file_tmp >= 0) {
        s = rank_tmp * 8 + file_tmp;
        SET_SQUARE(occupancy, s);
        if (GET_SQUARE(blockers, s)) {
            break;
        }
        rank_tmp--;
        file_tmp--;
    }

    // Calculate top left
    rank_tmp = rank + 1;
    file_tmp = file - 1;
    while (rank_tmp <= 7 && file_tmp >= 0) {
        s = rank_tmp * 8 + file_tmp;
        SET_SQUARE(occupancy, s);
        if (GET_SQUARE(blockers, s)) {
            break;
        }
        rank_tmp++;
        file_tmp--;
    }

    // Calculate top right
    rank_tmp = rank + 1;
    file_tmp = file + 1;
    while (rank_tmp <= 7 && file_tmp <= 7) {
        s = rank_tmp * 8 + file_tmp;
        SET_SQUARE(occupancy, s);
        if (GET_SQUARE(blockers, s)) {
            break;
        }
        rank_tmp++;
        file_tmp++;
    }

    // Calculate top right
    rank_tmp = rank - 1;
    file_tmp = file + 1;
    while (rank_tmp >= 0 && file_tmp <= 7) {
        s = rank_tmp * 8 + file_tmp;
        SET_SQUARE(occupancy, s);
        if (GET_SQUARE(blockers, s)) {
            break;
        }
        rank_tmp--;
        file_tmp++;
    }
    return occupancy;
}

U64 generate_rook_attacks(int square, U64 blockers)
{
    U64 occupancy = 0ULL;
    int s;

    int rank = square / 8;
    int file = square % 8;

    // Calculate left
    int rank_tmp = rank;
    int file_tmp = file - 1;
    while (file_tmp >= 0) {
        s = rank_tmp * 8 + file_tmp;
        SET_SQUARE(occupancy, s);
        if (GET_SQUARE(blockers, s)) {
            break;
        }
        file_tmp--;
    }

    // Calculate right
    rank_tmp = rank;
    file_tmp = file + 1;
    while (file_tmp <= 7) {
        s = rank_tmp * 8 + file_tmp;
        SET_SQUARE(occupancy, s);
        if (GET_SQUARE(blockers, s)) {
            break;
        }
        file_tmp++;
    }

    // Calculate top
    rank_tmp = rank + 1;
    file_tmp = file;
    while (rank_tmp <= 7) {
        s = rank_tmp * 8 + file_tmp;
        SET_SQUARE(occupancy, s);
        if (GET_SQUARE(blockers, s)) {
            break;
        }
        rank_tmp++;
    }

    // Calculate bottom
    rank_tmp = rank - 1;
    file_tmp = file;
    while (rank_tmp >= 0) {
        s = rank_tmp * 8 + file_tmp;
        SET_SQUARE(occupancy, s);
        if (GET_SQUARE(blockers, s)) {
            break;
        }
        rank_tmp--;
    }
    return occupancy;
}

inline U64 get_bishop_attacks(int square, U64 blockers)
{
    blockers &= bishop_occupancy[square];
    blockers *= bishop_magic[square];
    int magic_index = blockers >> (64 - bishop_bits[square]);

    return bishop_attacks[square][magic_index];
}

inline U64 get_rook_attacks(int square, U64 blockers)
{
    blockers &= rook_occupancy[square];
    blockers *= rook_magic[square];
    int magic_index = blockers >> (64 - rook_bits[square]);

    return rook_attacks[square][magic_index];
}

inline U64 get_queen_attacks(int square, U64 blockers)
{
    return get_bishop_attacks(square, blockers) | get_rook_attacks(square, blockers);
}

void init_sliders()
{
    int square;
    for (square = a1; square <= h8; square++) {
        bishop_occupancy[square] = calculate_bishop_occupancy(square);
        rook_occupancy[square] = calculate_rook_occupancy(square);

        bishop_magic[square] = find_magic_number(square, bishop_bits[square], 1);
        rook_magic[square] = find_magic_number(square, rook_bits[square], 0);
    }

    for (square = a1; square <= h8; square++) {
        for (int i = 0; i < (1 << bishop_bits[square]); i++) {
            U64 occupancy = occupancy_from_index(i, bishop_occupancy[square]);
            int magic_index = (occupancy * bishop_magic[square]) >> (64 - bishop_bits[square]);
            bishop_attacks[square][magic_index] = generate_bishop_attacks(square, occupancy);
        }

        for (int i = 0; i < (1 << rook_bits[square]); i++) {
            U64 occupancy = occupancy_from_index(i, rook_occupancy[square]);
            int magic_index = (occupancy * rook_magic[square]) >> (64 - rook_bits[square]);
            rook_attacks[square][magic_index] = generate_rook_attacks(square, occupancy);
        }
    }
}

void init_leapers()
{
    for (int square = a1; square <= h8; square++) {
        knight_attacks[square] = calculate_knight_attacks(square);
        king_attacks[square] = calculate_king_attacks(square);
        pawn_attacks[WHITE][square] = calculate_pawn_attacks(WHITE, square);
        pawn_attacks[BLACK][square] = calculate_pawn_attacks(BLACK, square);
    }
}

void init_attacks()
{
    init_sliders();
    init_leapers();
}

void print_bitboard(U64 board)
{
    printf("  +---+---+---+---+---+---+---+---+\n");
    for (int i = 7; i >= 0; i--) {
        printf("%d ", i + 1);
        for (int j = 0; j < 8; j++) {
            printf("| %d ", GET_SQUARE(board, i * 8 + j) ? 1 : 0);
        }
        printf("|\n");
        printf("  +---+---+---+---+---+---+---+---+\n");
    }
    printf("    a   b   c   d   e   f   g   h\n");
    printf("Bitboard: 0x%llx\n\n", board);
}
