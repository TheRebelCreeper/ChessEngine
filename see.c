#include "see.h"

#include "bitboard.h"
#include "move.h"
#include "position.h"

/*
	Static Exchange Evaluation
	see_at_least answers whether a move wins at least threshold material, tracking a
	running balance and negamaxing it after each recapture instead of building a swap list
	Values are separate from piece_value, which is signed by color and has a 10000 king
*/
static const int see_piece_value[13] = {100, 310, 330, 500, 900, 0, 100, 310, 330, 500, 900, 0, 0};

// Material won before any recapture
static int see_gain(const GameState *pos, Move move)
{
    // Castling only moves across unattacked squares
    if (IS_MOVE_CASTLES(move))
        return 0;

    // The victim of en passant is not on the destination square
    if (IS_MOVE_EP(move))
        return see_piece_value[P];

    int promotion = GET_MOVE_PROMOTION(move);
    int victim = GET_MOVE_CAPTURED(move);
    int value = (victim == NO_CAPTURE) ? 0 : see_piece_value[victim];
    if (promotion)
        value += see_piece_value[promotion] - see_piece_value[P];

    return value;
}

bool see_at_least(const GameState *pos, Move move, int threshold)
{
    int dst = GET_MOVE_DST(move);
    int promotion = GET_MOVE_PROMOTION(move);

    // The piece left on dst is what the opponent captures first
    int next_victim = promotion ? promotion : GET_MOVE_PIECE(move);

    // Best case is winning everything on offer with nothing coming back
    int balance = see_gain(pos, move) - threshold;
    if (balance < 0)
        return false;

    // Worst case is immediately losing the piece that just moved
    balance -= see_piece_value[next_victim];
    if (balance >= 0)
        return true;

    u64 bishops = pos->piece_bitboards[B] | pos->piece_bitboards[b]
                  | pos->piece_bitboards[Q] | pos->piece_bitboards[q];
    u64 rooks = pos->piece_bitboards[R] | pos->piece_bitboards[r]
                | pos->piece_bitboards[Q] | pos->piece_bitboards[q];

    // Occupancy as if the move was played
    u64 occupancy = pos->occupancies[BOTH];
    CLEAR_SQUARE(occupancy, GET_MOVE_SRC(move));
    SET_SQUARE(occupancy, dst);
    if (IS_MOVE_EP(move))
        CLEAR_SQUARE(occupancy, (pos->turn == WHITE) ? dst - 8 : dst + 8);

    u64 attackers = ((pawn_attacks[BLACK][dst] & pos->piece_bitboards[P])
                     | (pawn_attacks[WHITE][dst] & pos->piece_bitboards[p])
                     | (knight_attacks[dst] & (pos->piece_bitboards[N] | pos->piece_bitboards[n]))
                     | (king_attacks[dst] & (pos->piece_bitboards[K] | pos->piece_bitboards[k]))
                     | (get_bishop_attacks(dst, occupancy) & bishops)
                     | (get_rook_attacks(dst, occupancy) & rooks)) & occupancy;
    int turn = pos->turn ^ 1;

    while (true) {
        u64 our_attackers = attackers & pos->occupancies[turn];

        // Out of attackers, so the exchange cannot continue
        if (our_attackers == 0ULL)
            break;

        // Recapture with the least valuable piece
        int offset = 6 * turn;
        u64 piece_bb = 0ULL;
        for (next_victim = P; next_victim <= K; next_victim++) {
            piece_bb = our_attackers & pos->piece_bitboards[next_victim + offset];
            if (piece_bb)
                break;
        }

        CLEAR_SQUARE(occupancy, GET_FIRST_BIT_SQUARE(piece_bb));

        // Recomputing sliders lets x-rays join the exchange as it unfolds
        if (next_victim == P || next_victim == B || next_victim == Q)
            attackers |= get_bishop_attacks(dst, occupancy) & bishops;
        if (next_victim == R || next_victim == Q)
            attackers |= get_rook_attacks(dst, occupancy) & rooks;

        attackers &= occupancy;
        turn ^= 1;

        // Flip sides and give up the piece just used. The -1 makes the test below strict
        balance = -balance - 1 - see_piece_value[next_victim];

        if (balance >= 0) {
            // Capturing with the king into a defended square would be illegal
            if (next_victim == K && (attackers & pos->occupancies[turn]))
                turn ^= 1;
            break;
        }
    }

    // Whoever runs out of attackers first loses the exchange
    return pos->turn != turn;
}
