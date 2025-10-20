#include "movegen.h"
#include <string.h>
#include "bitboard.h"
#include "movelist.h"
#include "position.h"

void generate_pawn_moves(const GameState *pos, int turn, int piece_offset, MoveList *move_list)
{
    int i = move_list->next_open;
    int src, dst, piece = P + piece_offset;
    U64 single_push_target, double_push_target;
    U64 occupancy = pos->occupancies[BOTH];
    U64 enemy_pieces = (turn == WHITE) ? pos->occupancies[BLACK] : pos->occupancies[WHITE];

    // Generate pawn moves
    U64 piece_bb = pos->piece_bitboards[P + piece_offset];

    // Pawn pushes
    if (turn == WHITE) {
        single_push_target = (piece_bb << 8) & ~occupancy;
        double_push_target = (single_push_target << 8) & RANK_4 & ~occupancy;
    }
    else {
        single_push_target = (piece_bb >> 8) & ~occupancy;
        double_push_target = (single_push_target >> 8) & RANK_5 & ~occupancy;
    }

    while (double_push_target) {
        dst = GET_FIRST_BIT_SQUARE(double_push_target);
        src = dst - 16 + (32 * turn);
        move_list->move[i++] = CREATE_MOVE(src, dst, piece, NO_PROMOTION, NO_CAPTURE, 1, 0, 0);
        CLEAR_LSB(double_push_target);
    }

    while (single_push_target) {
        dst = GET_FIRST_BIT_SQUARE(single_push_target);
        src = dst - 8 + (16 * turn);
        if ((dst <= h8 && dst >= a8) || (dst >= a1 && dst <= h1)) {
            move_list->move[i++] = CREATE_MOVE(src, dst, piece, Q, NO_CAPTURE, 0, 0, 0);
            move_list->move[i++] = CREATE_MOVE(src, dst, piece, R, NO_CAPTURE, 0, 0, 0);
            move_list->move[i++] = CREATE_MOVE(src, dst, piece, B, NO_CAPTURE, 0, 0, 0);
            move_list->move[i++] = CREATE_MOVE(src, dst, piece, N, NO_CAPTURE, 0, 0, 0);
        }
        else {
            move_list->move[i++] = CREATE_MOVE(src, dst, piece, NO_PROMOTION, NO_CAPTURE, 0, 0, 0);
        }
        CLEAR_LSB(single_push_target);
    }

    // Pawn Captures
    while (piece_bb) {
        src = GET_FIRST_BIT_SQUARE(piece_bb);
        U64 piece_attacks = pawn_attacks[turn][src] & enemy_pieces;

        // EP captures
        if (pos->enpassant_square != none) {
            U64 epAttacks = pawn_attacks[turn][src] & (1ULL << pos->enpassant_square);
            if (epAttacks) {
                dst = pos->enpassant_square;
                move_list->move[i++] = CREATE_MOVE(src, dst, piece, NO_PROMOTION, p - piece_offset, 0, 1, 0);
            }
        }

        while (piece_attacks) {
            dst = GET_FIRST_BIT_SQUARE(piece_attacks);
            int victim = (pos->mailbox[dst] == NO_PIECE) ? NO_CAPTURE : pos->mailbox[dst];

            // Promotion
            if ((dst <= h8 && dst >= a8) || (dst >= a1 && dst <= h1)) {
                move_list->move[i++] = CREATE_MOVE(src, dst, piece, Q, victim, 0, 0, 0);
                move_list->move[i++] = CREATE_MOVE(src, dst, piece, R, victim, 0, 0, 0);
                move_list->move[i++] = CREATE_MOVE(src, dst, piece, B, victim, 0, 0, 0);
                move_list->move[i++] = CREATE_MOVE(src, dst, piece, N, victim, 0, 0, 0);
            }
            else {
                move_list->move[i++] = CREATE_MOVE(src, dst, piece, NO_PROMOTION, victim, 0, 0, 0);
            }
            CLEAR_LSB(piece_attacks);
        }
        CLEAR_LSB(piece_bb);
    }
    move_list->next_open = i;
}

void generate_king_moves(const GameState *pos, int turn, int piece_offset, MoveList *move_list)
{
    int i = move_list->next_open;
    int piece = K + piece_offset;
    U64 occupancy = pos->occupancies[BOTH];
    U64 friendly_pieces = pos->occupancies[turn];

    // Generate castling moves, does not support 960
    U64 piece_bb = pos->piece_bitboards[K + piece_offset];
    unsigned char castling_rights = pos->castling_rights;
    int src = GET_FIRST_BIT_SQUARE(piece_bb);
    if (turn == WHITE) {
        if (castling_rights & WHITE_OO && !GET_SQUARE(occupancy, f1) && !GET_SQUARE(occupancy, g1)) {
            if (!(is_square_attacked(pos, e1, BLACK) || is_square_attacked(pos, f1, BLACK))) {
                move_list->move[i++] = CREATE_MOVE(src, src + 2, K, NO_PROMOTION, NO_CAPTURE, 0, 0, 1);
            }
        }
        if (castling_rights & WHITE_OOO && !GET_SQUARE(occupancy, b1) && !GET_SQUARE(occupancy, c1) && !
            GET_SQUARE(occupancy, d1)) {
            if (!(is_square_attacked(pos, e1, BLACK) || is_square_attacked(pos, d1, BLACK))) {
                move_list->move[i++] = CREATE_MOVE(src, src - 2, K, NO_PROMOTION, NO_CAPTURE, 0, 0, 1);
            }
        }
    }
    else {
        if (castling_rights & BLACK_OO && !GET_SQUARE(occupancy, f8) && !GET_SQUARE(occupancy, g8)) {
            if (!(is_square_attacked(pos, e8, WHITE) || is_square_attacked(pos, f8, WHITE))) {
                move_list->move[i++] = CREATE_MOVE(src, src + 2, k, NO_PROMOTION, NO_CAPTURE, 0, 0, 1);
            }
        }
        if (castling_rights & BLACK_OOO && !GET_SQUARE(occupancy, b8) && !GET_SQUARE(occupancy, c8) && !
            GET_SQUARE(occupancy, d8)) {
            if (!(is_square_attacked(pos, e8, WHITE) || is_square_attacked(pos, d8, WHITE))) {
                move_list->move[i++] = CREATE_MOVE(src, src - 2, k, NO_PROMOTION, NO_CAPTURE, 0, 0, 1);
            }
        }
    }

    // Generate King Moves
    while (piece_bb) {
        src = GET_FIRST_BIT_SQUARE(piece_bb);
        U64 piece_attacks = king_attacks[src] & ~friendly_pieces;
        while (piece_attacks) {
            int dst = GET_FIRST_BIT_SQUARE(piece_attacks);
            int victim = (pos->mailbox[dst] == NO_PIECE) ? NO_CAPTURE : pos->mailbox[dst];
            move_list->move[i++] = CREATE_MOVE(src, dst, piece, NO_PROMOTION, victim, 0, 0, 0);
            CLEAR_LSB(piece_attacks);
        }
        CLEAR_LSB(piece_bb);
    }
    move_list->next_open = i;
}

void generate_knight_moves(const GameState *pos, int turn, int piece_offset, MoveList *move_list)
{
    int i = move_list->next_open;
    int piece = N + piece_offset;

    U64 friendly_pieces = pos->occupancies[turn];

    // Generate Knight Moves
    U64 piece_bb = pos->piece_bitboards[N + piece_offset];
    while (piece_bb) {
        int src = GET_FIRST_BIT_SQUARE(piece_bb);
        U64 piece_attacks = knight_attacks[src] & ~friendly_pieces;
        while (piece_attacks) {
            int dst = GET_FIRST_BIT_SQUARE(piece_attacks);
            int victim = (pos->mailbox[dst] == NO_PIECE) ? NO_CAPTURE : pos->mailbox[dst];
            move_list->move[i++] = CREATE_MOVE(src, dst, piece, NO_PROMOTION, victim, 0, 0, 0);
            CLEAR_LSB(piece_attacks);
        }
        CLEAR_LSB(piece_bb);
    }
    move_list->next_open = i;
}

void generate_bishop_moves(const GameState *pos, int turn, int piece_offset, MoveList *move_list)
{
    int i = move_list->next_open;
    int piece = B + piece_offset;

    U64 occupancy = pos->occupancies[BOTH];
    U64 friendly_pieces = pos->occupancies[turn];

    // Generate Bishop Moves
    U64 piece_bb = pos->piece_bitboards[B + piece_offset];
    while (piece_bb) {
        int src = GET_FIRST_BIT_SQUARE(piece_bb);
        U64 piece_attacks = get_bishop_attacks(src, occupancy) & ~friendly_pieces;
        while (piece_attacks) {
            int dst = GET_FIRST_BIT_SQUARE(piece_attacks);
            int victim = (pos->mailbox[dst] == NO_PIECE) ? NO_CAPTURE : pos->mailbox[dst];
            move_list->move[i++] = CREATE_MOVE(src, dst, piece, NO_PROMOTION, victim, 0, 0, 0);
            CLEAR_LSB(piece_attacks);
        }
        CLEAR_LSB(piece_bb);
    }
    move_list->next_open = i;
}

void generate_rook_moves(const GameState *pos, int turn, int piece_offset, MoveList *move_list)
{
    int i = move_list->next_open;
    int piece = R + piece_offset;

    U64 occupancy = pos->occupancies[BOTH];
    U64 friendly_pieces = pos->occupancies[turn];

    // Generate Rook Moves
    U64 piece_bb = pos->piece_bitboards[piece];
    while (piece_bb) {
        int src = GET_FIRST_BIT_SQUARE(piece_bb);
        U64 piece_attacks = get_rook_attacks(src, occupancy) & ~friendly_pieces;
        while (piece_attacks) {
            int dst = GET_FIRST_BIT_SQUARE(piece_attacks);
            int victim = (pos->mailbox[dst] == NO_PIECE) ? NO_CAPTURE : pos->mailbox[dst];
            move_list->move[i++] = CREATE_MOVE(src, dst, piece, NO_PROMOTION, victim, 0, 0, 0);
            CLEAR_LSB(piece_attacks);
        }
        CLEAR_LSB(piece_bb);
    }
    move_list->next_open = i;
}

void generate_queen_moves(const GameState *pos, int turn, int piece_offset, MoveList *move_list)
{
    int i = move_list->next_open;
    int piece = Q + piece_offset;

    U64 occupancy = pos->occupancies[BOTH];
    U64 friendly_pieces = pos->occupancies[turn];

    // Generate Queen Moves
    U64 piece_bb = pos->piece_bitboards[piece];
    while (piece_bb) {
        int src = GET_FIRST_BIT_SQUARE(piece_bb);
        U64 piece_attacks = get_queen_attacks(src, occupancy) & ~friendly_pieces;
        while (piece_attacks) {
            int dst = GET_FIRST_BIT_SQUARE(piece_attacks);
            int victim = (pos->mailbox[dst] == NO_PIECE) ? NO_CAPTURE : pos->mailbox[dst];
            move_list->move[i++] = CREATE_MOVE(src, dst, piece, NO_PROMOTION, victim, 0, 0, 0);
            CLEAR_LSB(piece_attacks);
        }
        CLEAR_LSB(piece_bb);
    }
    move_list->next_open = i;
}

void generate_pawn_noisy(const GameState *pos, int turn, int piece_offset, MoveList *move_list)
{
    int i = move_list->next_open;
    int src, dst, piece = P + piece_offset;
    U64 single_push_target;
    U64 occupancy = pos->occupancies[BOTH];
    U64 enemy_pieces = (turn == WHITE) ? pos->occupancies[BLACK] : pos->occupancies[WHITE];

    // Generate pawn moves
    U64 piece_bb = pos->piece_bitboards[P + piece_offset];

    // Pawn pushes
    if (turn == WHITE) {
        single_push_target = (piece_bb << 8) & ~occupancy;
    }
    else {
        single_push_target = (piece_bb >> 8) & ~occupancy;
    }

    // Only generate promotions
    while (single_push_target) {
        dst = GET_FIRST_BIT_SQUARE(single_push_target);
        src = dst - 8 + (16 * turn);
        if ((dst <= h8 && dst >= a8) || (dst >= a1 && dst <= h1)) {
            move_list->move[i++] = CREATE_MOVE(src, dst, piece, Q, NO_CAPTURE, 0, 0, 0);
            move_list->move[i++] = CREATE_MOVE(src, dst, piece, R, NO_CAPTURE, 0, 0, 0);
            move_list->move[i++] = CREATE_MOVE(src, dst, piece, B, NO_CAPTURE, 0, 0, 0);
            move_list->move[i++] = CREATE_MOVE(src, dst, piece, N, NO_CAPTURE, 0, 0, 0);
        }
        CLEAR_LSB(single_push_target);
    }

    // Pawn Captures
    while (piece_bb) {
        src = GET_FIRST_BIT_SQUARE(piece_bb);
        U64 piece_attacks = pawn_attacks[turn][src] & enemy_pieces;

        // EP captures
        if (pos->enpassant_square != none) {
            U64 epAttacks = pawn_attacks[turn][src] & (1ULL << pos->enpassant_square);
            if (epAttacks) {
                dst = pos->enpassant_square;
                move_list->move[i++] = CREATE_MOVE(src, dst, piece, NO_PROMOTION, p - piece_offset, 0, 1, 0);
            }
        }

        while (piece_attacks) {
            dst = GET_FIRST_BIT_SQUARE(piece_attacks);
            int victim = pos->mailbox[dst];

            // Promotion
            if ((dst <= h8 && dst >= a8) || (dst >= a1 && dst <= h1)) {
                move_list->move[i++] = CREATE_MOVE(src, dst, piece, Q, victim, 0, 0, 0);
                move_list->move[i++] = CREATE_MOVE(src, dst, piece, R, victim, 0, 0, 0);
                move_list->move[i++] = CREATE_MOVE(src, dst, piece, B, victim, 0, 0, 0);
                move_list->move[i++] = CREATE_MOVE(src, dst, piece, N, victim, 0, 0, 0);
            }
            else {
                move_list->move[i++] = CREATE_MOVE(src, dst, piece, NO_PROMOTION, victim, 0, 0, 0);
            }
            CLEAR_LSB(piece_attacks);
        }
        CLEAR_LSB(piece_bb);
    }
    move_list->next_open = i;
}

void generate_knight_noisy(const GameState *pos, int turn, int piece_offset, MoveList *move_list)
{
    int i = move_list->next_open;
    int piece = N + piece_offset;

    U64 enemy_pieces = (turn == WHITE) ? pos->occupancies[BLACK] : pos->occupancies[WHITE];

    // Generate Knight Moves
    U64 piece_bb = pos->piece_bitboards[N + piece_offset];
    while (piece_bb) {
        int src = GET_FIRST_BIT_SQUARE(piece_bb);
        U64 piece_attacks = knight_attacks[src] & enemy_pieces;
        while (piece_attacks) {
            int dst = GET_FIRST_BIT_SQUARE(piece_attacks);
            int victim = pos->mailbox[dst];
            move_list->move[i++] = CREATE_MOVE(src, dst, piece, NO_PROMOTION, victim, 0, 0, 0);
            CLEAR_LSB(piece_attacks);
        }
        CLEAR_LSB(piece_bb);
    }
    move_list->next_open = i;
}

void generate_bishop_noisy(const GameState *pos, int turn, int piece_offset, MoveList *move_list)
{
    int i = move_list->next_open;
    int piece = B + piece_offset;

    U64 occupancy = pos->occupancies[BOTH];
    U64 enemy_pieces = (turn == WHITE) ? pos->occupancies[BLACK] : pos->occupancies[WHITE];

    // Generate Bishop Moves
    U64 piece_bb = pos->piece_bitboards[B + piece_offset];
    while (piece_bb) {
        int src = GET_FIRST_BIT_SQUARE(piece_bb);
        U64 piece_attacks = get_bishop_attacks(src, occupancy) & enemy_pieces;
        while (piece_attacks) {
            int dst = GET_FIRST_BIT_SQUARE(piece_attacks);
            int victim = pos->mailbox[dst];
            move_list->move[i++] = CREATE_MOVE(src, dst, piece, NO_PROMOTION, victim, 0, 0, 0);
            CLEAR_LSB(piece_attacks);
        }
        CLEAR_LSB(piece_bb);
    }
    move_list->next_open = i;
}

void generate_rook_noisy(const GameState *pos, int turn, int piece_offset, MoveList *move_list)
{
    int i = move_list->next_open;
    int piece = R + piece_offset;

    U64 occupancy = pos->occupancies[BOTH];
    U64 enemy_pieces = (turn == WHITE) ? pos->occupancies[BLACK] : pos->occupancies[WHITE];

    // Generate Rook Moves
    U64 piece_bb = pos->piece_bitboards[piece];
    while (piece_bb) {
        int src = GET_FIRST_BIT_SQUARE(piece_bb);
        U64 piece_attacks = get_rook_attacks(src, occupancy) & enemy_pieces;
        while (piece_attacks) {
            int dst = GET_FIRST_BIT_SQUARE(piece_attacks);
            int victim = pos->mailbox[dst];
            move_list->move[i++] = CREATE_MOVE(src, dst, piece, NO_PROMOTION, victim, 0, 0, 0);
            CLEAR_LSB(piece_attacks);
        }
        CLEAR_LSB(piece_bb);
    }
    move_list->next_open = i;
}

void generate_queen_noisy(const GameState *pos, int turn, int piece_offset, MoveList *move_list)
{
    int i = move_list->next_open;
    int piece = Q + piece_offset;

    U64 occupancy = pos->occupancies[BOTH];
    U64 enemy_pieces = (turn == WHITE) ? pos->occupancies[BLACK] : pos->occupancies[WHITE];

    // Generate Queen Moves
    U64 piece_bb = pos->piece_bitboards[piece];
    while (piece_bb) {
        int src = GET_FIRST_BIT_SQUARE(piece_bb);
        U64 piece_attacks = get_queen_attacks(src, occupancy) & enemy_pieces;
        while (piece_attacks) {
            int dst = GET_FIRST_BIT_SQUARE(piece_attacks);
            int victim = pos->mailbox[dst];
            move_list->move[i++] = CREATE_MOVE(src, dst, piece, NO_PROMOTION, victim, 0, 0, 0);
            CLEAR_LSB(piece_attacks);
        }
        CLEAR_LSB(piece_bb);
    }
    move_list->next_open = i;
}

void generate_king_noisy(const GameState *pos, int turn, int piece_offset, MoveList *move_list)
{
    int i = move_list->next_open;
    int piece = K + piece_offset;
    U64 enemy_pieces = (turn == WHITE) ? pos->occupancies[BLACK] : pos->occupancies[WHITE];

    U64 piece_bb = pos->piece_bitboards[K + piece_offset];

    // Generate King Moves
    while (piece_bb) {
        int src = GET_FIRST_BIT_SQUARE(piece_bb);
        U64 piece_attacks = king_attacks[src] & enemy_pieces;
        while (piece_attacks) {
            int dst = GET_FIRST_BIT_SQUARE(piece_attacks);
            int victim = pos->mailbox[dst];
            move_list->move[i++] = CREATE_MOVE(src, dst, piece, NO_PROMOTION, victim, 0, 0, 0);
            CLEAR_LSB(piece_attacks);
        }
        CLEAR_LSB(piece_bb);
    }
    move_list->next_open = i;
}

MoveList generate_moves_qsearch(const GameState *pos, int *size)
{
    MoveList move_list;
    move_list.next_open = 0;
    memset(move_list.move, 0, sizeof(Move) * MAX_MOVES);
    int turn = pos->turn;
    int piece_offset = 6 * turn;

    generate_pawn_noisy(pos, turn, piece_offset, &move_list);
    generate_knight_noisy(pos, turn, piece_offset, &move_list);
    generate_bishop_noisy(pos, turn, piece_offset, &move_list);
    generate_rook_noisy(pos, turn, piece_offset, &move_list);
    generate_queen_noisy(pos, turn, piece_offset, &move_list);
    generate_king_noisy(pos, turn, piece_offset, &move_list);

    // Maybe add an assert to make sure size isn't greater than MAX_MOVES
    *size = move_list.next_open;
    return move_list;
}

MoveList generate_moves(const GameState *pos, int *size)
{
    MoveList move_list;
    move_list.next_open = 0;
    memset(move_list.move, 0, sizeof(Move) * MAX_MOVES);
    int turn = pos->turn;
    int piece_offset = 6 * turn;

    generate_pawn_moves(pos, turn, piece_offset, &move_list);
    generate_knight_moves(pos, turn, piece_offset, &move_list);
    generate_bishop_moves(pos, turn, piece_offset, &move_list);
    generate_rook_moves(pos, turn, piece_offset, &move_list);
    generate_queen_moves(pos, turn, piece_offset, &move_list);
    generate_king_moves(pos, turn, piece_offset, &move_list);

    // Maybe add an assert to make sure size isn't greater than MAX_MOVES
    *size = move_list.next_open;
    return move_list;
}
