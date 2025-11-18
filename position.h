#ifndef POSITION_H
#define POSITION_H

#include <stdbool.h>

#include "bitboard.h"

#define DELIMS "/ "
#define BOTH 2

#define WHITE_OO 8
#define WHITE_OOO 4
#define BLACK_OO 2
#define BLACK_OOO 1

#define STARTING_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

typedef struct gameState {
    U64 piece_bitboards[12];
    U64 occupancies[3];
    U64 key;
    int turn;
    unsigned char castling_rights;
    unsigned char mailbox[64];
    int enpassant_square;
    int half_move_clock;
    int full_move;
} GameState;

extern const char *promotion_notation[12];
extern const int piece_lookup[2][6];

U64 piece_keys[12][64];
U64 castle_keys[16];
U64 epKey[8];
U64 side_key;

U64 generate_pos_key(const GameState *pos);
int get_square_from_notation(const char *str);
void load_fen(GameState *state, const char *fen);
void set_occupancies(GameState *pos);
void init_keys();

inline int get_piece_at_square(const GameState *pos, int square)
{
    return pos->mailbox[square];
}

/*
inline int is_endgame(GameState *pos)
{
    // Material balance (excluding kings)
    int whiteMaterial = 0;
    int blackMaterial = 0;

    // Pawns
    whiteMaterial += 100 * COUNT_BITS(pos->piece_bitboards[P]);
    blackMaterial += 100 * COUNT_BITS(pos->piece_bitboards[p]);

    // Knights
    whiteMaterial += 320 * COUNT_BITS(pos->piece_bitboards[N]);
    blackMaterial += 320 * COUNT_BITS(pos->piece_bitboards[n]);

    // Bishops
    whiteMaterial += 330 * COUNT_BITS(pos->piece_bitboards[B]);
    blackMaterial += 330 * COUNT_BITS(pos->piece_bitboards[b]);

    // Rooks
    whiteMaterial += 500 * COUNT_BITS(pos->piece_bitboards[R]);
    blackMaterial += 500 * COUNT_BITS(pos->piece_bitboards[r]);

    // Queens
    whiteMaterial += 900 * COUNT_BITS(pos->piece_bitboards[Q]);
    blackMaterial += 900 * COUNT_BITS(pos->piece_bitboards[q]);

    int totalMaterial = whiteMaterial + blackMaterial;

    // Rule of thumb:
    // Endgame when queens are off OR when remaining material (excluding pawns) is very low
    int queens = COUNT_BITS(pos->piece_bitboards[Q]) + COUNT_BITS(pos->piece_bitboards[q]);
    int heavyPieces = COUNT_BITS(pos->piece_bitboards[R]) + COUNT_BITS(pos->piece_bitboards[r]) +
                      COUNT_BITS(pos->piece_bitboards[Q]) + COUNT_BITS(pos->piece_bitboards[q]);

    if (queens == 0) return 1;
    if (heavyPieces <= 2 && totalMaterial <= 2400) return 1;

    return 0;
} */


inline bool is_endgame(const GameState *pos)
{
    return COUNT_BITS(pos->occupancies[BOTH]) <= 10;
}

inline bool only_has_pawns(const GameState *pos, int side)
{
    int piece = (side == WHITE) ? P : p;
    int total_pieces = COUNT_BITS(pos->occupancies[side]);
    int total_pawns = COUNT_BITS(pos->piece_bitboards[piece]);
    return total_pieces - 1 == total_pawns;
}

inline bool insufficient_material(const GameState *pos)
{
    int total_pieces = COUNT_BITS(pos->occupancies[BOTH]);
    int white_knights = COUNT_BITS(pos->piece_bitboards[N]);
    int white_bishops = COUNT_BITS(pos->piece_bitboards[B]);
    int black_knights = COUNT_BITS(pos->piece_bitboards[n]);
    int black_bishops = COUNT_BITS(pos->piece_bitboards[b]);
    int minors = white_knights + white_bishops + black_knights + black_bishops;

    // Not insufficient_material
    if (total_pieces - minors > 2)
        return false;

    // King vs King
    if (total_pieces == 2)
        return true;

    // K+N vs K or K+B vs K
    if (minors == 1)
        return true;

    if (minors == 2 && white_knights == 1 && black_knights == 1)
        return true;
    return false;
}

inline bool is_square_attacked(const GameState *pos, int square, int by_color)
{
    int offset = 6 * by_color;
    int pawn_attack_color = by_color ^ 1;
    U64 occupancy = pos->occupancies[BOTH];

    if (king_attacks[square] & pos->piece_bitboards[K + offset])
        return true;
    if (knight_attacks[square] & pos->piece_bitboards[N + offset])
        return true;
    if (pawn_attacks[pawn_attack_color][square] & pos->piece_bitboards[P + offset])
        return true;
    if (get_bishop_attacks(square, occupancy) & (pos->piece_bitboards[B + offset] | pos->piece_bitboards[
                                                     Q + offset]))
        return true;
    if (get_rook_attacks(square, occupancy) & (pos->piece_bitboards[R + offset] | pos->piece_bitboards[
                                                   Q + offset]))
        return true;

    return false;
}

inline int get_smallest_attacker(const GameState *pos, int square)
{
    int offset = 6 * pos->turn;
    int pawn_attack_color = pos->turn ^ 1;
    U64 occupancy = pos->occupancies[BOTH];

    if (pawn_attacks[pawn_attack_color][square] & pos->piece_bitboards[P + offset])
        return P + offset;
    if (knight_attacks[square] & pos->piece_bitboards[N + offset])
        return N + offset;
    if (get_bishop_attacks(square, occupancy) & pos->piece_bitboards[B + offset])
        return B + offset;
    if (get_rook_attacks(square, occupancy) & pos->piece_bitboards[R + offset])
        return R + offset;
    if (get_queen_attacks(square, occupancy) & pos->piece_bitboards[Q + offset])
        return Q + offset;
    if (king_attacks[square] & pos->piece_bitboards[K + offset])
        return K + offset;

    return -1;
}

inline bool is_in_check(const GameState *pos)
{
    int king_location = GET_FIRST_BIT_SQUARE(pos->piece_bitboards[piece_lookup[pos->turn][K]]);
    return is_square_attacked(pos, king_location, pos->turn ^ 1);
}

void print_board(GameState state);

#endif
