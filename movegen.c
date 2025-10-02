#include "movegen.h"
#include <string.h>
#include "bitboard.h"
#include "list.h"
#include "position.h"

void generatePawnMoves(GameState *pos, int turn, int piece_offset, MoveList *move_list)
{
    int i = move_list->next_open;
    int src, dst, piece = P + piece_offset;
    U64 pieceBB, pieceAttacks, enemyPieces, occupancy;
    U64 singlePushTarget, doublePushTarget;
    occupancy = pos->occupancies[BOTH];
    enemyPieces = (turn == WHITE) ? pos->occupancies[BLACK] : pos->occupancies[WHITE];

    // Generate pawn moves
    pieceBB = pos->pieceBitboards[P + piece_offset];

    // Pawn pushes
    if (turn == WHITE) {
        singlePushTarget = (pieceBB << 8) & ~occupancy;
        doublePushTarget = (singlePushTarget << 8) & Rank4 & ~occupancy;
    }
    else {
        singlePushTarget = (pieceBB >> 8) & ~occupancy;
        doublePushTarget = (singlePushTarget >> 8) & Rank5 & ~occupancy;
    }

    while (doublePushTarget) {
        dst = getFirstBitSquare(doublePushTarget);
        src = dst - 16 + (32 * turn);
        move_list->list[i++] = CREATE_MOVE(src, dst, piece, NO_PROMOTION, NO_CAPTURE, 1, 0, 0);
        clear_lsb(doublePushTarget);
    }

    while (singlePushTarget) {
        dst = getFirstBitSquare(singlePushTarget);
        src = dst - 8 + (16 * turn);
        if ((dst <= h8 && dst >= a8) || (dst >= a1 && dst <= h1)) {
            move_list->list[i++] = CREATE_MOVE(src, dst, piece, Q, NO_CAPTURE, 0, 0, 0);
            move_list->list[i++] = CREATE_MOVE(src, dst, piece, R, NO_CAPTURE, 0, 0, 0);
            move_list->list[i++] = CREATE_MOVE(src, dst, piece, B, NO_CAPTURE, 0, 0, 0);
            move_list->list[i++] = CREATE_MOVE(src, dst, piece, N, NO_CAPTURE, 0, 0, 0);
        }
        else {
            move_list->list[i++] = CREATE_MOVE(src, dst, piece, NO_PROMOTION, NO_CAPTURE, 0, 0, 0);
        }
        clear_lsb(singlePushTarget);
    }

    // Pawn Captures
    while (pieceBB) {
        src = getFirstBitSquare(pieceBB);
        pieceAttacks = pawnAttacks[turn][src] & enemyPieces;

        // EP captures
        if (pos->enpassantSquare != none) {
            U64 epAttacks = pawnAttacks[turn][src] & (1ULL << pos->enpassantSquare);
            if (epAttacks) {
                dst = pos->enpassantSquare;
                move_list->list[i++] = CREATE_MOVE(src, dst, piece, NO_PROMOTION, P, 0, 1, 0);
            }
        }

        while (pieceAttacks) {
            dst = getFirstBitSquare(pieceAttacks);
            int victim = NO_CAPTURE;
            int offset2 = (piece_offset == 0) ? 6 : 0;
            for (int j = P; j <= K; j++) {
                if (get_square(pos->pieceBitboards[j + offset2], dst)) {
                    victim = j;
                    break;
                }
            }

            // Promotion
            if ((dst <= h8 && dst >= a8) || (dst >= a1 && dst <= h1)) {
                move_list->list[i++] = CREATE_MOVE(src, dst, piece, Q, victim, 0, 0, 0);
                move_list->list[i++] = CREATE_MOVE(src, dst, piece, R, victim, 0, 0, 0);
                move_list->list[i++] = CREATE_MOVE(src, dst, piece, B, victim, 0, 0, 0);
                move_list->list[i++] = CREATE_MOVE(src, dst, piece, N, victim, 0, 0, 0);
            }
            else {
                move_list->list[i++] = CREATE_MOVE(src, dst, piece, NO_PROMOTION, victim, 0, 0, 0);
            }
            clear_lsb(pieceAttacks);
        }
        clear_lsb(pieceBB);
    }
    move_list->next_open = i;
}

void generateKingMoves(GameState *pos, int turn, int piece_offset, MoveList *move_list)
{
    int i = move_list->next_open;
    int src, dst, piece = K + piece_offset;
    U64 pieceBB, pieceAttacks, friendlyPieces, occupancy;
    occupancy = pos->occupancies[BOTH];
    friendlyPieces = pos->occupancies[turn];

    pieceBB = pos->pieceBitboards[K + piece_offset];
    unsigned char castlingRights = pos->castlingRights;
    src = getFirstBitSquare(pieceBB);
    if (turn == WHITE) {
        if (castlingRights & WHITE_OO && !get_square(occupancy, f1) && !get_square(occupancy, g1)) {
            if (!(isSquareAttacked(pos, e1, BLACK) || isSquareAttacked(pos, f1, BLACK))) {
                move_list->list[i++] = CREATE_MOVE(src, src + 2, K, NO_PROMOTION, NO_CAPTURE, 0, 0, 1);
            }
        }
        if (castlingRights & WHITE_OOO && !get_square(occupancy, b1) && !get_square(occupancy, c1) && !
            get_square(occupancy, d1)) {
            if (!(isSquareAttacked(pos, e1, BLACK) || isSquareAttacked(pos, d1, BLACK))) {
                move_list->list[i++] = CREATE_MOVE(src, src - 2, K, NO_PROMOTION, NO_CAPTURE, 0, 0, 1);
            }
        }
    }
    else {
        if (castlingRights & BLACK_OO && !get_square(occupancy, f8) && !get_square(occupancy, g8)) {
            if (!(isSquareAttacked(pos, e8, WHITE) || isSquareAttacked(pos, f8, WHITE))) {
                move_list->list[i++] = CREATE_MOVE(src, src + 2, k, NO_PROMOTION, NO_CAPTURE, 0, 0, 1);
            }
        }
        if (castlingRights & BLACK_OOO && !get_square(occupancy, b8) && !get_square(occupancy, c8) && !
            get_square(occupancy, d8)) {
            if (!(isSquareAttacked(pos, e8, WHITE) || isSquareAttacked(pos, d8, WHITE))) {
                move_list->list[i++] = CREATE_MOVE(src, src - 2, k, NO_PROMOTION, NO_CAPTURE, 0, 0, 1);
            }
        }
    }

    // Generate King Moves
    while (pieceBB) {
        src = getFirstBitSquare(pieceBB);
        pieceAttacks = kingAttacks[src] & ~friendlyPieces;
        while (pieceAttacks) {
            dst = getFirstBitSquare(pieceAttacks);
            int victim = NO_CAPTURE;
            if (get_square(pos->occupancies[BOTH], dst)) {
                int offset2 = (piece_offset == 0) ? 6 : 0;
                for (int j = P; j <= K; j++) {
                    if (get_square(pos->pieceBitboards[j + offset2], dst)) {
                        victim = j;
                        break;
                    }
                }
            }
            move_list->list[i++] = CREATE_MOVE(src, dst, piece, NO_PROMOTION, victim, 0, 0, 0);
            clear_lsb(pieceAttacks);
        }
        clear_lsb(pieceBB);
    }
    move_list->next_open = i;
}

void generateKnightMoves(GameState *pos, int turn, int piece_offset, MoveList *move_list)
{
    int i = move_list->next_open;
    int src, dst, piece = N + piece_offset;

    U64 pieceBB, pieceAttacks, friendlyPieces;
    friendlyPieces = pos->occupancies[turn];

    // Generate Knight Moves
    pieceBB = pos->pieceBitboards[N + piece_offset];
    while (pieceBB) {
        src = getFirstBitSquare(pieceBB);
        pieceAttacks = knightAttacks[src] & ~friendlyPieces;
        while (pieceAttacks) {
            int victim = NO_CAPTURE;
            dst = getFirstBitSquare(pieceAttacks);
            if (get_square(pos->occupancies[2], dst)) {
                int offset2 = (piece_offset == 0) ? 6 : 0;
                for (int j = P; j <= K; j++) {
                    if (get_square(pos->pieceBitboards[j + offset2], dst)) {
                        victim = j;
                        break;
                    }
                }
            }
            move_list->list[i++] = CREATE_MOVE(src, dst, piece, NO_PROMOTION, victim, 0, 0, 0);
            clear_lsb(pieceAttacks);
        }
        clear_lsb(pieceBB);
    }
    move_list->next_open = i;
}

void generateBishopMoves(GameState *pos, int turn, int piece_offset, MoveList *move_list)
{
    int i = move_list->next_open;
    int src, dst, piece = B + piece_offset;

    U64 pieceBB, pieceAttacks, friendlyPieces, occupancy;
    occupancy = pos->occupancies[BOTH];
    friendlyPieces = pos->occupancies[turn];

    // Generate Bishop Moves
    pieceBB = pos->pieceBitboards[B + piece_offset];
    while (pieceBB) {
        src = getFirstBitSquare(pieceBB);
        pieceAttacks = getBishopAttacks(src, occupancy) & ~friendlyPieces;
        while (pieceAttacks) {
            int victim = NO_CAPTURE;
            dst = getFirstBitSquare(pieceAttacks);
            if (get_square(pos->occupancies[2], dst)) {
                int offset2 = (piece_offset == 0) ? 6 : 0;
                for (int j = P; j <= K; j++) {
                    if (get_square(pos->pieceBitboards[j + offset2], dst)) {
                        victim = j;
                        break;
                    }
                }
            }
            move_list->list[i++] = CREATE_MOVE(src, dst, piece, NO_PROMOTION, victim, 0, 0, 0);
            clear_lsb(pieceAttacks);
        }
        clear_lsb(pieceBB);
    }
    move_list->next_open = i;
}

void generateRookMoves(GameState *pos, int turn, int piece_offset, MoveList *move_list)
{
    int i = move_list->next_open;
    int src, dst, piece = R + piece_offset;

    U64 pieceBB, pieceAttacks, friendlyPieces, occupancy;
    occupancy = pos->occupancies[BOTH];
    friendlyPieces = pos->occupancies[turn];

    // Generate Rook Moves
    pieceBB = pos->pieceBitboards[piece];
    while (pieceBB) {
        src = getFirstBitSquare(pieceBB);
        pieceAttacks = getRookAttacks(src, occupancy) & ~friendlyPieces;
        while (pieceAttacks) {
            dst = getFirstBitSquare(pieceAttacks);
            int victim = NO_CAPTURE;
            if (get_square(pos->occupancies[2], dst)) {
                int offset2 = (piece_offset == 0) ? 6 : 0;
                for (int j = P; j <= K; j++) {
                    if (get_square(pos->pieceBitboards[j + offset2], dst)) {
                        victim = j;
                        break;
                    }
                }
            }
            move_list->list[i++] = CREATE_MOVE(src, dst, piece, NO_PROMOTION, victim, 0, 0, 0);
            clear_lsb(pieceAttacks);
        }
        clear_lsb(pieceBB);
    }
    move_list->next_open = i;
}

void generateQueenMoves(GameState *pos, int turn, int piece_offset, MoveList *move_list)
{
    int i = move_list->next_open;
    int src, dst, piece = Q + piece_offset;

    U64 pieceBB, pieceAttacks, friendlyPieces, occupancy;
    occupancy = pos->occupancies[BOTH];
    friendlyPieces = pos->occupancies[turn];

    // Generate Queen Moves
    pieceBB = pos->pieceBitboards[piece];
    while (pieceBB) {
        src = getFirstBitSquare(pieceBB);
        pieceAttacks = getQueenAttacks(src, occupancy) & ~friendlyPieces;
        while (pieceAttacks) {
            int victim = NO_CAPTURE;
            dst = getFirstBitSquare(pieceAttacks);
            if (get_square(pos->occupancies[2], dst)) {
                int offset2 = (piece_offset == 0) ? 6 : 0;
                for (int j = P; j <= K; j++) {
                    if (get_square(pos->pieceBitboards[j + offset2], dst)) {
                        victim = j;
                        break;
                    }
                }
            }
            move_list->list[i++] = CREATE_MOVE(src, dst, piece, NO_PROMOTION, victim, 0, 0, 0);
            clear_lsb(pieceAttacks);
        }
        clear_lsb(pieceBB);
    }
    move_list->next_open = i;
}

MoveList generateMoves(GameState *pos, int *size)
{
    MoveList move_list;
    move_list.next_open = 0;
    memset(move_list.list, 0, sizeof(Move) * MAX_MOVES);
    int turn = pos->turn;
    int piece_offset = 6 * turn;

    generatePawnMoves(pos, turn, piece_offset, &move_list);
    generateKnightMoves(pos, turn, piece_offset, &move_list);
    generateBishopMoves(pos, turn, piece_offset, &move_list);
    generateRookMoves(pos, turn, piece_offset, &move_list);
    generateKingMoves(pos, turn, piece_offset, &move_list);
    generateQueenMoves(pos, turn, piece_offset, &move_list);

    // Maybe add an assert to make sure size isn't greater than MAX_MOVES
    *size = move_list.next_open;
    return move_list;
}
