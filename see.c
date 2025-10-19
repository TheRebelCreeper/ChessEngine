#include "see.h"

#include <stdlib.h>

#include "evaluation.h"
#include "position.h"

int see(const GameState *pos, int square)
{
    U64 occupied = pos->occupancies[BOTH]; // all pieces
    U64 attackers[2]; // white & black attackers
    int gain[32]; // swap values
    int depth = 0;

    // piece on the target square (initial capture value)
    int captured = pos->mailbox[square];
    if (captured == NO_PIECE) {
        return 0;
    }

    // Initialize attackers
    attackers[WHITE] = pawn_attacks[BLACK][square] & pos->piece_bitboards[P];
    attackers[WHITE] |= knight_attacks[square] & pos->piece_bitboards[N];
    attackers[WHITE] |= get_bishop_attacks(square, occupied) & (pos->piece_bitboards[B] | pos->piece_bitboards[Q]);
    attackers[WHITE] |= get_rook_attacks(square, occupied) & (pos->piece_bitboards[R] | pos->piece_bitboards[Q]);
    attackers[WHITE] |= king_attacks[square] & pos->piece_bitboards[K];

    attackers[BLACK] = pawn_attacks[WHITE][square] & pos->piece_bitboards[p];
    attackers[BLACK] |= knight_attacks[square] & pos->piece_bitboards[n];
    attackers[BLACK] |= get_bishop_attacks(square, occupied) & (pos->piece_bitboards[b] | pos->piece_bitboards[q]);
    attackers[BLACK] |= get_rook_attacks(square, occupied) & (pos->piece_bitboards[r] | pos->piece_bitboards[q]);
    attackers[BLACK] |= king_attacks[square] & pos->piece_bitboards[k];

    // swap list holds material balance after each exchange
    gain[depth] = abs(piece_value[captured]);
    //printf("gain[%d] = %d\n", depth, gain[depth]);
    int stm = pos->turn; // side to move

    do {
        // find least valuable attacker for current side
        int attacker = NO_PIECE;
        int attacker_sq = -1;
        int min_value = 99999;
        int offset = stm * 6;

        for (int i = P; i <= K; i++) {
            U64 bb = pos->piece_bitboards[i + offset] & attackers[stm];
            if (bb) {
                int sq = GET_FIRST_BIT_SQUARE(bb);
                int val = abs(piece_value[i + offset]);
                if (val < min_value) {
                    min_value = val;
                    attacker = i + offset;
                    attacker_sq = sq;
                }
            }
        }

        if (attacker == NO_PIECE) {
            break;
        }

        // next captured piece is the value of this attacker
        depth++;
        gain[depth] = -gain[depth - 1] + abs(piece_value[attacker]);
        //printf("gain[%d] = %d\n", depth, gain[depth]);

        // remove attacker from board
        CLEAR_SQUARE(occupied, attacker_sq);

        // update attackers (sliders may now attack through)
        attackers[WHITE] = (attackers[WHITE] & occupied) |
                           (get_bishop_attacks(square, occupied) & (pos->piece_bitboards[B] | pos->piece_bitboards[Q]) &
                            occupied) |
                           (get_rook_attacks(square, occupied) & (pos->piece_bitboards[R] | pos->piece_bitboards[Q]) &
                            occupied);

        attackers[BLACK] = (attackers[BLACK] & occupied) |
                           (get_bishop_attacks(square, occupied) & (pos->piece_bitboards[b] | pos->piece_bitboards[q]) &
                            occupied) |
                           (get_rook_attacks(square, occupied) & (pos->piece_bitboards[r] | pos->piece_bitboards[q]) &
                            occupied);

        stm = !stm; // switch side
    }
    while (depth < 31); // safety bound

    // minimax from the end
    while (--depth) {
        if (-gain[depth] < gain[depth - 1])
            gain[depth - 1] = -gain[depth];
    }

    return gain[0];
}
