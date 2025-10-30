#include "move.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "bitboard.h"
#include "position.h"

/*
	There are special moves for kings and pawns only
	For kings, 1 = O-O, 2 = O-O-O
	For pawns, 4 = Q, 3 = R, 2 = B, 1 = N
*/


unsigned char adjust_castling_rights(const GameState *pos, int src, int dst, int piece)
{
    unsigned char castling_rights = pos->castling_rights;
    if (src == a1 || dst == a1) {
        castling_rights &= (WHITE_OO | BLACK_OO | BLACK_OOO);
    }

    if (src == h1 || dst == h1) {
        castling_rights &= (WHITE_OOO | BLACK_OO | BLACK_OOO);
    }

    if (src == a8 || dst == a8) {
        castling_rights &= (BLACK_OO | WHITE_OO | WHITE_OOO);
    }

    if (src == h8 || dst == h8) {
        castling_rights &= (BLACK_OOO | WHITE_OO | WHITE_OOO);
    }

    if (piece == K) {
        castling_rights &= (BLACK_OO | BLACK_OOO);
    }

    if (piece == k) {
        castling_rights &= (WHITE_OO | WHITE_OOO);
    }
    return castling_rights;
}

int is_noisy(Move move)
{
    assert(move != 0);
    return move & (IS_CAPTURE | IS_PROMOTION | IS_CASTLES);
}

int play_move(const GameState *old_pos, GameState *new_pos, Move move)
{
    memcpy(new_pos, old_pos, sizeof(GameState));
    int turn = old_pos->turn;
    int piece = GET_MOVE_PIECE(move);
    int src = GET_MOVE_SRC(move);
    int dst = GET_MOVE_DST(move);
    int promotion = GET_MOVE_PROMOTION(move);
    int offset = 0;
    U64 hash_key = old_pos->key;

    if (turn == BLACK) {
        offset = 6;
        new_pos->full_move += 1;
    }

    new_pos->turn ^= 1;
    new_pos->half_move_clock += 1;

    // Clear Source
    hash_key ^= side_key;
    hash_key ^= piece_keys[piece][src];
    CLEAR_SQUARE(new_pos->piece_bitboards[piece], src);
    CLEAR_SQUARE(new_pos->occupancies[turn], src);
    new_pos->mailbox[src] = NO_PIECE;

    // En Passant Moves
    if ((piece == P || piece == p) && IS_MOVE_EP(move)) {
        hash_key ^= epKey[old_pos->enpassant_square & 7];
        if (turn == WHITE) {
            hash_key ^= piece_keys[p][dst - 8];
            CLEAR_SQUARE(new_pos->piece_bitboards[p], dst - 8);
            CLEAR_SQUARE(new_pos->occupancies[BLACK], dst - 8);
            new_pos->mailbox[dst - 8] = NO_PIECE;
        }
        else {
            hash_key ^= piece_keys[P][dst + 8];
            CLEAR_SQUARE(new_pos->piece_bitboards[P], dst + 8);
            CLEAR_SQUARE(new_pos->occupancies[WHITE], dst + 8);
            new_pos->mailbox[dst + 8] = NO_PIECE;
        }
    }

    // Clear Destination
    CLEAR_SQUARE(new_pos->occupancies[new_pos->turn], dst);
    int victim = GET_MOVE_CAPTURED(move);
    if (victim != NO_CAPTURE) {
        CLEAR_SQUARE(new_pos->piece_bitboards[victim], dst);
        hash_key ^= piece_keys[victim][dst];
    }

    // Set destination
    // If pawn promotion
    SET_SQUARE(new_pos->occupancies[turn], dst);
    if (piece == (P + offset) && promotion) {
        hash_key ^= piece_keys[promotion + offset][dst];
        SET_SQUARE(new_pos->piece_bitboards[promotion + offset], dst);
        new_pos->mailbox[dst] = promotion + offset;
    }
    else {
        hash_key ^= piece_keys[piece][dst];
        SET_SQUARE(new_pos->piece_bitboards[piece], dst);
        new_pos->mailbox[dst] = piece;
    }

    // Castling
    if (piece == (K + offset) && IS_MOVE_CASTLES(move)) {
        //Short Castling
        if (src < dst) {
            hash_key ^= piece_keys[R + offset][dst + 1];
            CLEAR_SQUARE(new_pos->piece_bitboards[R + offset], dst + 1);
            CLEAR_SQUARE(new_pos->occupancies[turn], dst + 1);
            new_pos->mailbox[dst + 1] = NO_PIECE;

            hash_key ^= piece_keys[R + offset][dst - 1];
            SET_SQUARE(new_pos->piece_bitboards[R + offset], dst - 1);
            SET_SQUARE(new_pos->occupancies[turn], dst - 1);
            new_pos->mailbox[dst - 1] = R + offset;
        }
        // Long Castling
        else if (src > dst) {
            hash_key ^= piece_keys[R + offset][dst - 2];
            CLEAR_SQUARE(new_pos->piece_bitboards[R + offset], dst - 2);
            CLEAR_SQUARE(new_pos->occupancies[turn], dst - 2);
            new_pos->mailbox[dst - 2] = NO_PIECE;

            hash_key ^= piece_keys[R + offset][dst + 1];
            SET_SQUARE(new_pos->piece_bitboards[R + offset], dst + 1);
            SET_SQUARE(new_pos->occupancies[turn], dst + 1);
            new_pos->mailbox[dst + 1] = R + offset;
        }
    }

    // Unset old castling rights, then set new ones
    hash_key ^= castle_keys[new_pos->castling_rights];
    new_pos->occupancies[BOTH] = new_pos->occupancies[WHITE] | new_pos->occupancies[BLACK];
    new_pos->castling_rights = (old_pos->castling_rights) ? adjust_castling_rights(old_pos, src, dst, piece) : 0;
    hash_key ^= castle_keys[new_pos->castling_rights];

    // Reset 50 move counter if capture or pawn push
    if (GET_MOVE_CAPTURED(move) != NO_CAPTURE || piece == P || piece == p) {
        new_pos->half_move_clock = 0;
    }

    new_pos->enpassant_square = none;
    if (IS_MOVE_DPP(move)) {
        new_pos->enpassant_square = src + 8 - (16 * turn);
        hash_key ^= epKey[new_pos->enpassant_square & 7];
    }

    new_pos->key = hash_key;

    // Legality Check
    int king_location = GET_FIRST_BIT_SQUARE(new_pos->piece_bitboards[K + offset]);
    return !is_square_attacked(new_pos, king_location, new_pos->turn);
}

void print_move(Move m)
{
    int promotion = GET_MOVE_PROMOTION(m);
    if (m) {
        printf("%s%s%s", square_names[GET_MOVE_SRC(m)], square_names[GET_MOVE_DST(m)], piece_notation[promotion]);
    }
    else {
        printf("0000");
    }
}
