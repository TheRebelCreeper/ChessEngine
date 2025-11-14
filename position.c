#include "position.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "magic.h"
#include "pcg.h"


char *piece_chars[13] = {"P", "N", "B", "R", "Q", "K", "p", "n", "b", "r", "q", "k", " "};
char *piece_notation[12] = {"", "n", "b", "r", "q", "k", "", "n", "b", "r", "q", "k"};

int piece_lookup[2][6] =
{
    {P, N, B, R, Q, K},
    {p, n, b, r, q, k}
};

int get_square_from_notation(const char *str)
{
    if (strlen(str) != 2) {
        printf("Invalid square\n");
        exit(EXIT_FAILURE);
    }
    return (str[1] - '0' - 1) * 8 + (tolower(str[0]) - 'a');
}

int get_piece_from_char(char c)
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
    return NO_PIECE;
}

char get_castling_rights(const char *str)
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

void set_occupancies(GameState *pos)
{
    pos->occupancies[WHITE] = 0ULL;
    pos->occupancies[BLACK] = 0ULL;

    pos->occupancies[WHITE] |= pos->piece_bitboards[P];
    pos->occupancies[WHITE] |= pos->piece_bitboards[N];
    pos->occupancies[WHITE] |= pos->piece_bitboards[B];
    pos->occupancies[WHITE] |= pos->piece_bitboards[R];
    pos->occupancies[WHITE] |= pos->piece_bitboards[Q];
    pos->occupancies[WHITE] |= pos->piece_bitboards[K];

    pos->occupancies[BLACK] |= pos->piece_bitboards[p];
    pos->occupancies[BLACK] |= pos->piece_bitboards[n];
    pos->occupancies[BLACK] |= pos->piece_bitboards[b];
    pos->occupancies[BLACK] |= pos->piece_bitboards[r];
    pos->occupancies[BLACK] |= pos->piece_bitboards[q];
    pos->occupancies[BLACK] |= pos->piece_bitboards[k];

    pos->occupancies[BOTH] = pos->occupancies[WHITE] | pos->occupancies[BLACK];
}

void init_keys()
{
    pcg32_srandom(42u, 54u);
    side_key = random_u64();
    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 64; j++) {
            piece_keys[i][j] = random_u64();
        }
    }

    for (int i = 0; i < 16; i++) {
        castle_keys[i] = random_u64();
    }

    for (int i = 0; i < 8; i++) {
        epKey[i] = random_u64();
    }
}

U64 generate_pos_key(const GameState *pos)
{
    U64 final_key = 0ULL;

    for (int i = P; i <= k; i++) {
        // Generate Knight Moves
        U64 piece_bb = pos->piece_bitboards[i];
        while (piece_bb) {
            int src = GET_FIRST_BIT_SQUARE(piece_bb);
            final_key ^= piece_keys[i][src];
            CLEAR_LSB(piece_bb);
        }
    }

    if (pos->turn == BLACK) {
        final_key ^= side_key;
    }

    if (pos->enpassant_square != none) {
        final_key ^= epKey[pos->enpassant_square & 7];
    }

    final_key ^= castle_keys[pos->castling_rights];

    return final_key;
}

void load_fen(GameState *state, const char *fen)
{
    char *str = NULL;
    char *token = NULL;

    int length = strlen(fen);
    str = malloc(length + 1);
    if (str == NULL) {
        perror("Malloc error in load_fen");
        exit(EXIT_FAILURE);
    }
    memset(str, 0, length + 1);
    memcpy(str, fen, length);
    str[length] = 0;

    memset(state->piece_bitboards, 0ULL, sizeof(state->piece_bitboards));
    memset(state->occupancies, 0ULL, sizeof(state->occupancies));
    memset(state->mailbox, NO_PIECE, sizeof(state->mailbox));
    state->turn = 0;
    state->castling_rights = 0;
    state->enpassant_square = none;
    state->half_move_clock = 0;
    state->full_move = 1;

    token = strtok(str, DELIMS);
    // For loops read in pieces
    for (int rank = 7; rank >= 0; rank--) {
        int i = 0;
        for (int file = 0; file < 8; file++) {
            int piece = get_piece_from_char(token[i]);
            if (isdigit(token[i])) {
                file += (token[i] - '0') - 1;
            }
            else {
                if (piece == NO_PIECE) {
                    printf("Invalid FEN\n");
                    exit(EXIT_FAILURE);
                }
                int square = rank * 8 + file;
                SET_SQUARE(state->piece_bitboards[piece], square);
                state->mailbox[square] = piece;
            }
            i++;
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
    state->castling_rights = get_castling_rights(token);

    // Get enpassant_square
    token = strtok(NULL, DELIMS);
    state->enpassant_square = (token[0] == '-') ? none : get_square_from_notation(token);

    token = strtok(NULL, DELIMS);
    state->half_move_clock = (token) ? atoi(token) : 0;
    token = strtok(NULL, DELIMS);
    state->full_move = (token) ? atoi(token) : 1;

    set_occupancies(state);

    state->key = generate_pos_key(state);

    free(str);
}

void print_board(GameState state)
{
    printf("  +---+---+---+---+---+---+---+---+\n");
    for (int rank = 0; rank < 8; rank++) {
        if (state.turn == WHITE)
            printf("%d ", 8 - rank);
        else
            printf("%d ", rank + 1);

        for (int file = 0; file < 8; file++) {
            int square = (state.turn == WHITE) ? ((7 - rank) * 8 + file) : (rank * 8 + (7 - file));
            char *piece = piece_chars[state.mailbox[square]];

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
    if (state.castling_rights & WHITE_OO)
        printf("K");
    if (state.castling_rights & WHITE_OOO)
        printf("Q");
    if (state.castling_rights & BLACK_OO)
        printf("k");
    if (state.castling_rights & BLACK_OOO)
        printf("q");
    if (!state.castling_rights)
        printf("-");
    printf("\n");
    printf("En Passant Square: %s\n", square_names[state.enpassant_square]);
    printf("Halfmove Clock: %d\n", state.half_move_clock);
    printf("Move: %d\n", state.full_move);
    printf("Hash Key: %llx\n", state.key);
}
