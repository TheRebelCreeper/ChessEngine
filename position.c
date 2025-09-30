#include "position.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "magic.h"

int flip = 0;

int historyIndex = 0;

char *pieceChars[13] = {"P", "N", "B", "R", "Q", "K", "p", "n", "b", "r", "q", "k", " "};
char *pieceNotation[12] = {"", "n", "b", "r", "q", "k", "", "n", "b", "r", "q", "k"};

int pieceLookup[2][6] =
{
    {P, N, B, R, Q, K},
    {p, n, b, r, q, k}
};

int getSquareFromNotation(char *str)
{
    if (strlen(str) != 2) {
        printf("Invalid square\n");
        exit(EXIT_FAILURE);
    }
    return (str[1] - '0' - 1) * 8 + (tolower(str[0]) - 'a');
}

int getPieceFromChar(char c)
{
    if (c == 'P')
        return P;
    if (c == 'N')
        return N;
    if (c == 'B')
        return B;
    if (c == 'R')
        return R;
    if (c == 'Q')
        return Q;
    if (c == 'K')
        return K;
    if (c == 'p')
        return p;
    if (c == 'n')
        return n;
    if (c == 'b')
        return b;
    if (c == 'r')
        return r;
    if (c == 'q')
        return q;
    if (c == 'k')
        return k;
    else
        return NO_PIECE;
}

char getCastlingRights(char *str)
{
    char rights = 0;
    if (strlen(str) > 4) {
        printf("Invalid FEN\n");
        exit(EXIT_FAILURE);
    }
    if (strchr(str, 'K') != NULL) {
        rights |= WHITE_OO;
    }
    if (strchr(str, 'Q') != NULL) {
        rights |= WHITE_OOO;
    }
    if (strchr(str, 'k') != NULL) {
        rights |= BLACK_OO;
    }
    if (strchr(str, 'q') != NULL) {
        rights |= BLACK_OOO;
    }

    return rights;
}

void initKeys()
{
    sideKey = random_u64();
    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 64; j++) {
            pieceKeys[i][j] = random_u64();
        }
    }

    for (int i = 0; i < 16; i++) {
        castleKeys[i] = random_u64();
    }

    for (int i = 0; i < 8; i++) {
        epKey[i] = random_u64();
    }
}

U64 generatePosKey(GameState *pos)
{
    int src;
    int i;
    U64 finalKey = 0ULL, pieceBB;

    for (i = P; i <= k; i++) {
        // Generate Knight Moves
        pieceBB = pos->pieceBitboards[i];
        while (pieceBB) {
            src = getFirstBitSquare(pieceBB);
            finalKey ^= pieceKeys[i][src];
            clear_lsb(pieceBB);
        }
    }

    if (pos->turn == BLACK) {
        finalKey ^= sideKey;
    }

    if (pos->enpassantSquare != no_square) {
        finalKey ^= epKey[pos->enpassantSquare & 7];
    }

    finalKey ^= castleKeys[pos->castlingRights];

    return finalKey;
}

void loadFEN(GameState *state, char *fen)
{
    int rank, file, square, piece, index, length;
    char *str = NULL;
    char *token = NULL;

    length = strlen(fen);
    str = malloc(length + 1);
    if (str == NULL) {
        perror("Malloc error in loadFEN");
        exit(EXIT_FAILURE);
    }
    memset(str, 0, length + 1);
    memcpy(str, fen, length);
    str[length] = 0;

    memset(state->pieceBitboards, 0ULL, sizeof(state->pieceBitboards));
    memset(state->occupancies, 0ULL, sizeof(state->occupancies));
    state->turn = 0;
    state->castlingRights = 0;
    state->enpassantSquare = no_square;
    state->halfMoveClock = 0;
    state->fullMove = 1;

    token = strtok(str, DELIMS);
    // For loops read in pieces
    for (rank = 7; rank >= 0; rank--) {
        index = 0;
        for (file = 0; file < 8; file++) {
            piece = getPieceFromChar(token[index]);
            if (isdigit(token[index])) {
                file += (token[index] - '0') - 1;
            }
            else {
                if (piece == NO_PIECE) {
                    printf("Invalid FEN\n");
                    exit(EXIT_FAILURE);
                }
                square = rank * 8 + file;
                set_square(state->pieceBitboards[piece], square);
            }
            index++;
        }
        token = strtok(NULL, DELIMS);
    }

    // First gamestate is side to move
    if (strlen(token) == 1 && (token[0] == 'w' || token[0] == 'W')) {
        state->turn = WHITE;
    }
    else if (strlen(token) == 1 && (token[0] == 'b' || token[0] == 'B')) {
        state->turn = BLACK;
    }
    else {
        printf("Invalid FEN\n");
        exit(EXIT_FAILURE);
    }

    // Get castling gamestate
    token = strtok(NULL, DELIMS);
    state->castlingRights = getCastlingRights(token);

    // Get enpassantSquare
    token = strtok(NULL, DELIMS);
    state->enpassantSquare = (token[0] == '-') ? no_square : getSquareFromNotation(token);

    token = strtok(NULL, DELIMS);
    state->halfMoveClock = (token) ? atoi(token) : 0;
    token = strtok(NULL, DELIMS);
    state->fullMove = (token) ? atoi(token) : 1;

    setOccupancies(state);

    state->key = generatePosKey(state);

    free(str);
}

void printBoard(GameState state)
{
    char *piece;
    int rank, file, square;
    printf("  +---+---+---+---+---+---+---+---+\n");
    for (rank = 0; rank < 8; rank++) {
        if (state.turn == WHITE)
            printf("%d ", 8 - rank);
        else
            printf("%d ", rank + 1);

        for (file = 0; file < 8; file++) {
            square = (state.turn == WHITE) ? ((7 - rank) * 8 + file) : (rank * 8 + (7 - file));
            piece = pieceChars[getPieceAtSquare(&state, square)];

            printf("|");
#ifndef _WIN32
            if ((!(file & 1) && !(rank & 1)) || ((file & 1) && (rank & 1)))
                printf("\033[38;2;0;0;0;48;2;245;245;220m");
            else
                printf("\033[38;2;0;0;0;48;2;152;118;84m");
#endif
            printf(" %s ", piece);

#ifndef _WIN32
            printf("\033[39;49m");
#endif
        }
        printf("|\n");
        printf("  +---+---+---+---+---+---+---+---+\n");
    }
    if (state.turn == WHITE)
        printf("    a   b   c   d   e   f   g   h\n");
    else
        printf("    h   g   f   e   d   c   b   a\n");

    printf("\n%s to move\n", (state.turn == WHITE) ? "White" : "Black");
    printf("Castling Rights: ");
    if (state.castlingRights & WHITE_OO)
        printf("K");
    if (state.castlingRights & WHITE_OOO)
        printf("Q");
    if (state.castlingRights & BLACK_OO)
        printf("k");
    if (state.castlingRights & BLACK_OOO)
        printf("q");
    if (!state.castlingRights)
        printf("-");
    printf("\n");
    printf("En Passant Square: %s\n", squareNames[state.enpassantSquare]);
    printf("Halfmove Clock: %d\n", state.halfMoveClock);
    printf("Move: %d\n", state.fullMove);
    printf("Hash Key: %llx\n", state.key);
}
