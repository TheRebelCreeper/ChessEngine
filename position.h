#ifndef POSITION_H
#define POSITION_H

#include "bitboard.h"

#define DELIMS "/ "
#define BOTH 2

#define WHITE_OO 8
#define WHITE_OOO 4
#define BLACK_OO 2
#define BLACK_OOO 1

#define STARTING_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

typedef struct gameState {
    U64 pieceBitboards[12];
    U64 occupancies[3];
    U64 key;
    int turn;
    unsigned char castlingRights;
    int enpassantSquare;
    int halfMoveClock;
    int fullMove;
} GameState;

enum {
    P, N, B, R, Q, K, p, n, b, r, q, k, NO_PIECE
};

extern int historyIndex;
extern char *pieceNotation[12];
extern int pieceLookup[2][6];

U64 posHistory[256];
U64 pieceKeys[12][64];
U64 castleKeys[16];
U64 epKey[8];
U64 sideKey;

U64 generatePosKey(GameState *pos);
int getSquareFromNotation(char *str);
void loadFEN(GameState *state, char *fen);
void initKeys();

inline void setOccupancies(GameState *pos)
{
    pos->occupancies[WHITE] = 0ULL;
    pos->occupancies[BLACK] = 0ULL;

    pos->occupancies[WHITE] |= pos->pieceBitboards[P];
    pos->occupancies[WHITE] |= pos->pieceBitboards[N];
    pos->occupancies[WHITE] |= pos->pieceBitboards[B];
    pos->occupancies[WHITE] |= pos->pieceBitboards[R];
    pos->occupancies[WHITE] |= pos->pieceBitboards[Q];
    pos->occupancies[WHITE] |= pos->pieceBitboards[K];

    pos->occupancies[BLACK] |= pos->pieceBitboards[p];
    pos->occupancies[BLACK] |= pos->pieceBitboards[n];
    pos->occupancies[BLACK] |= pos->pieceBitboards[b];
    pos->occupancies[BLACK] |= pos->pieceBitboards[r];
    pos->occupancies[BLACK] |= pos->pieceBitboards[q];
    pos->occupancies[BLACK] |= pos->pieceBitboards[k];

    pos->occupancies[BOTH] = pos->occupancies[WHITE] | pos->occupancies[BLACK];
}

inline int getPieceAtSquare(GameState *pos, int square)
{
    for (int i = P; i <= k; i++) {
        if (get_square(pos->pieceBitboards[i], square))
            return i;
    }
    return NO_PIECE;
}

/*
inline int isEndgame(GameState *pos)
{
    // Material balance (excluding kings)
    int whiteMaterial = 0;
    int blackMaterial = 0;

    // Pawns
    whiteMaterial += 100 * countBits(pos->pieceBitboards[P]);
    blackMaterial += 100 * countBits(pos->pieceBitboards[p]);

    // Knights
    whiteMaterial += 320 * countBits(pos->pieceBitboards[N]);
    blackMaterial += 320 * countBits(pos->pieceBitboards[n]);

    // Bishops
    whiteMaterial += 330 * countBits(pos->pieceBitboards[B]);
    blackMaterial += 330 * countBits(pos->pieceBitboards[b]);

    // Rooks
    whiteMaterial += 500 * countBits(pos->pieceBitboards[R]);
    blackMaterial += 500 * countBits(pos->pieceBitboards[r]);

    // Queens
    whiteMaterial += 900 * countBits(pos->pieceBitboards[Q]);
    blackMaterial += 900 * countBits(pos->pieceBitboards[q]);

    int totalMaterial = whiteMaterial + blackMaterial;

    // Rule of thumb:
    // Endgame when queens are off OR when remaining material (excluding pawns) is very low
    int queens = countBits(pos->pieceBitboards[Q]) + countBits(pos->pieceBitboards[q]);
    int heavyPieces = countBits(pos->pieceBitboards[R]) + countBits(pos->pieceBitboards[r]) +
                      countBits(pos->pieceBitboards[Q]) + countBits(pos->pieceBitboards[q]);

    if (queens == 0) return 1;
    if (heavyPieces <= 2 && totalMaterial <= 2400) return 1;

    return 0;
} */


inline int isEndgame(GameState *pos)
{
    return countBits(pos->occupancies[BOTH]) <= 10;
}

inline int onlyHasPawns(GameState *pos, int side)
{
    int piece = (side == WHITE) ? P : p;
    int totalPieces = countBits(pos->occupancies[side]);
    int totalPawns = countBits(pos->pieceBitboards[piece]);
    return totalPieces - 1 == totalPawns;
}

inline int insufficientMaterial(GameState *pos)
{
    int totalPieces = countBits(pos->occupancies[BOTH]);
    int whiteKnights = countBits(pos->pieceBitboards[N]);
    int whiteBishops = countBits(pos->pieceBitboards[B]);
    int blackKnights = countBits(pos->pieceBitboards[n]);
    int blackBishops = countBits(pos->pieceBitboards[b]);
    int minors = whiteKnights + whiteBishops + blackKnights + blackBishops;

    // Not insufficientMaterial
    if (totalPieces - minors > 2)
        return 0;

    // King vs King
    if (totalPieces == 2)
        return 1;

    // K+N vs K or K+B vs K
    if (minors == 1)
        return 1;

    if (minors == 2 && whiteKnights == 1 && blackKnights == 1)
        return 1;
    else
        return 0;
}

inline int isSquareAttacked(GameState *pos, int square, int byColor)
{
    int colorOffset = 6 * byColor;
    int pawnAttackColor = byColor ^ 1;
    U64 occupancy = pos->occupancies[BOTH];

    if (kingAttacks[square] & pos->pieceBitboards[K + colorOffset])
        return 1;
    if (knightAttacks[square] & pos->pieceBitboards[N + colorOffset])
        return 1;
    if (pawnAttacks[pawnAttackColor][square] & pos->pieceBitboards[P + colorOffset])
        return 1;
    if (getBishopAttacks(square, occupancy) & (pos->pieceBitboards[B + colorOffset] | pos->pieceBitboards[
                                                   Q + colorOffset]))
        return 1;
    if (getRookAttacks(square, occupancy) & (pos->pieceBitboards[R + colorOffset] | pos->pieceBitboards[
                                                 Q + colorOffset]))
        return 1;

    return 0;
}

inline int get_smallest_attacker(GameState *pos, int square)
{
    int colorOffset = 6 * pos->turn;
    int pawnAttackColor = pos->turn ^ 1;
    U64 occupancy = pos->occupancies[BOTH];

    if (pawnAttacks[pawnAttackColor][square] & pos->pieceBitboards[P + colorOffset])
        return P + colorOffset;
    if (knightAttacks[square] & pos->pieceBitboards[N + colorOffset])
        return N + colorOffset;
    if (getBishopAttacks(square, occupancy) & pos->pieceBitboards[B + colorOffset])
        return B + colorOffset;
    if (getRookAttacks(square, occupancy) & pos->pieceBitboards[R + colorOffset])
        return R + colorOffset;
    if (getQueenAttacks(square, occupancy) & pos->pieceBitboards[Q + colorOffset])
        return Q + colorOffset;
    if (kingAttacks[square] & pos->pieceBitboards[K + colorOffset])
        return K + colorOffset;

    return -1;
}

inline int isInCheck(GameState *pos)
{
    int kingLocation = getFirstBitSquare(pos->pieceBitboards[pieceLookup[pos->turn][K]]);
    return isSquareAttacked(pos, kingLocation, pos->turn ^ 1);
}

void printBoard(GameState state);

#endif
