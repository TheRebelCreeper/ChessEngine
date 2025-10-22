#include "movelist.h"
#include "movepick.h"

#include "history.h"
#include "search.h"
#include "see.h"
#include "util.h"

void pick_move(MoveList *moves, int start_index)
{
    int best_index = start_index;
    int best_score = moves->score[start_index];
    for (int i = start_index; i < moves->next_open; i++) {
        if (moves->score[i] > best_score) {
            best_score = moves->score[i];
            best_index = i;
        }
    }

    // Swap moves if better score found
    if (best_index != start_index) {
        Move tmp_move = moves->move[start_index];
        moves->move[start_index] = moves->move[best_index];
        moves->move[best_index] = tmp_move;

        int tmp_score = moves->score[start_index];
        moves->score[start_index] = moves->score[best_index];
        moves->score[best_index] = tmp_score;
    }
}

void score_moves(MoveList *moves, const GameState *pos, Move tt_move, SearchInfo *info)
{
    for (int i = 0; i < moves->next_open; i++) {
        Move m = moves->move[i];
        int dst = GET_MOVE_DST(m);

        // Score TT hits
        if (tt_move != 0 && m == tt_move) {
            moves->score[i] = TT_HIT_SCORE;
            continue;
        }

        // Score captures
        if (m & IS_CAPTURE) {
            int piece_offset = 6 * pos->turn;
            moves->score[i] = MVV_LVA_TABLE[GET_MOVE_PIECE(m) - piece_offset][GET_MOVE_CAPTURED(m)] + KILLER_ONE;

            // Give bad score to results with negative SEE
            if (see(pos, dst) < -100) {
                moves->score[i] -= KILLER_ONE;
            }
        }
        // Score quiet moves
        else {
            moves->score[i] = get_history(pos, m);
        }
    }
}
