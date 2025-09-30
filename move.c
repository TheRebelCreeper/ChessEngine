#include "move.h"
#include <stdio.h>
#include <string.h>
#include "bitboard.h"
#include "position.h"

/*
	There are special moves for kings and pawns only
	For kings, 1 = O-O, 2 = O-O-O
	For pawns, 4 = Q, 3 = R, 2 = B, 1 = N
*/


unsigned char adjustCastlingRights(GameState *pos, int src, int dst, int piece)
{
    unsigned char castlingRights = pos->castlingRights;
    if (src == a1 || dst == a1) {
        castlingRights &= (WHITE_OO | BLACK_OO | BLACK_OOO);
    }

    if (src == h1 || dst == h1) {
        castlingRights &= (WHITE_OOO | BLACK_OO | BLACK_OOO);
    }

    if (src == a8 || dst == a8) {
        castlingRights &= (BLACK_OO | WHITE_OO | WHITE_OOO);
    }

    if (src == h8 || dst == h8) {
        castlingRights &= (BLACK_OOO | WHITE_OO | WHITE_OOO);
    }

    if (piece == K) {
        castlingRights &= (BLACK_OO | BLACK_OOO);
    }

    if (piece == k) {
        castlingRights &= (WHITE_OO | WHITE_OOO);
    }
    return castlingRights;
}

GameState playMove(GameState *pos, Move move, int *isLegal)
{
    GameState newPos;
    memcpy(&newPos, pos, sizeof(GameState));
    int turn = pos->turn;
    int piece = GET_MOVE_PIECE(move);
    int src = GET_MOVE_SRC(move);
    int dst = GET_MOVE_DST(move);
    int promotion = GET_MOVE_PROMOTION(move);
    int piece_offset = 6 * turn;
    U64 hashKey = pos->key;

    // Update newPos
    newPos.turn ^= 1;
    newPos.halfMoveClock += 1;
    newPos.enpassantSquare = no_square;
    hashKey ^= sideKey;

    // Clear Source
    hashKey ^= pieceKeys[piece][src];
    clear_square(newPos.pieceBitboards[piece], src);
    clear_square(newPos.occupancies[turn], src);

    // En Passant Moves
    if ((piece == P || piece == p) && IS_MOVE_EP(move)) {
        hashKey ^= epKey[pos->enpassantSquare & 7];
        if (turn == WHITE) {
            hashKey ^= pieceKeys[p][dst - 8];
            clear_square(newPos.pieceBitboards[p], dst - 8);
            clear_square(newPos.occupancies[BLACK], dst - 8);
        }
        else {
            hashKey ^= pieceKeys[P][dst + 8];
            clear_square(newPos.pieceBitboards[P], dst + 8);
            clear_square(newPos.occupancies[WHITE], dst + 8);
        }
    }

    // Clear Destination
    if (turn == WHITE) {
        clear_square(newPos.occupancies[BLACK], dst);
        int victim = GET_MOVE_CAPTURED(move);
        if (victim != NO_CAPTURE) {
            hashKey ^= pieceKeys[victim + 6][dst];
            clear_square(newPos.pieceBitboards[victim + 6], dst);
        }
    }
    else {
        clear_square(newPos.occupancies[WHITE], dst);
        int victim = GET_MOVE_CAPTURED(move);
        if (victim != NO_CAPTURE) {
            hashKey ^= pieceKeys[victim][dst];
            clear_square(newPos.pieceBitboards[victim], dst);
        }
        newPos.fullMove += 1;
    }

    // Set destination
    set_square(newPos.occupancies[turn], dst);

    // If pawn promotion
    if (piece == (P + piece_offset) && promotion) {
        hashKey ^= pieceKeys[promotion + piece_offset][dst];
        set_square(newPos.pieceBitboards[promotion + piece_offset], dst);
    }
    else {
        hashKey ^= pieceKeys[piece][dst];
        set_square(newPos.pieceBitboards[piece], dst);
    }

    // Castling
    if (piece == (K + piece_offset) && IS_MOVE_CASTLES(move)) {
        //Short Castling
        if (src < dst) {
            hashKey ^= pieceKeys[R + piece_offset][dst + 1];
            clear_square(newPos.pieceBitboards[R + piece_offset], dst + 1);
            clear_square(newPos.occupancies[turn], dst + 1);

            hashKey ^= pieceKeys[R + piece_offset][dst - 1];
            set_square(newPos.pieceBitboards[R + piece_offset], dst - 1);
            set_square(newPos.occupancies[turn], dst - 1);
        }
        // Long Castling
        else if (src > dst) {
            hashKey ^= pieceKeys[R + piece_offset][dst - 2];
            clear_square(newPos.pieceBitboards[R + piece_offset], dst - 2);
            clear_square(newPos.occupancies[turn], dst - 2);

            hashKey ^= pieceKeys[R + piece_offset][dst + 1];
            set_square(newPos.pieceBitboards[R + piece_offset], dst + 1);
            set_square(newPos.occupancies[turn], dst + 1);
        }
    }

    // Unset castle key, recalculate castling rights, set new castle key
    hashKey ^= castleKeys[newPos.castlingRights];
    newPos.occupancies[BOTH] = newPos.occupancies[WHITE] | newPos.occupancies[BLACK];
    newPos.castlingRights = (pos->castlingRights) ? adjustCastlingRights(pos, src, dst, piece) : 0;
    hashKey ^= castleKeys[newPos.castlingRights];

    // Reset 50 move counter if capture or pawn push
    if (GET_MOVE_CAPTURED(move) != NO_CAPTURE || piece == P || piece == p) {
        newPos.halfMoveClock = 0;
    }
    // Set EP square when a pawn moves 2 squares
    if (IS_MOVE_DPP(move)) {
        newPos.enpassantSquare = src + 8 - (16 * turn);
        hashKey ^= epKey[newPos.enpassantSquare & 7];
    }

    newPos.key = hashKey;

    // Legality Check
    int kingLocation = getFirstBitSquare(newPos.pieceBitboards[K + piece_offset]);
    *isLegal = !isSquareAttacked(&newPos, kingLocation, newPos.turn);

    return newPos;
}

inline GameState playNullMove(GameState *pos)
{
    GameState newPos;
    memcpy(&newPos, pos, sizeof(GameState));
    newPos.turn ^= 1;
    newPos.key ^= sideKey;

    // En Passant Moves
    if (pos->enpassantSquare != no_square) {
        newPos.enpassantSquare = no_square;
        newPos.key ^= epKey[pos->enpassantSquare & 7];
    }
    return newPos;
}

void printMove(Move m)
{
    int promotion = GET_MOVE_PROMOTION(m);
    if (m) {
        printf("%s%s%s", squareNames[GET_MOVE_SRC(m)], squareNames[GET_MOVE_DST(m)],
               (m) ? pieceNotation[promotion] : "");
    }
    else {
        printf("0000");
    }
}
