#include "wrapper.h"
#include "nnue.h"

int FOUND_NETWORK = 0;

void initNNUE(char *file)
{
    FOUND_NETWORK = nnue_init(file);
}

int evaluateNNUE(int turn, int *pieces, int *squares)
{
    return nnue_evaluate(turn, pieces, squares);
}

int evaluateFromFen(char *fen)
{
    return nnue_evaluate_fen(fen);
}
