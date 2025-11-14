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

void score_moves(MoveList *moves, const GameState *pos, Move tt_move, int ply)
{
    for (int i = 0; i < moves->next_open; i++) {
        moves->score[i] = HISTORY_SCORE_MAX;
    }
}
