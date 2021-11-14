#include "nnue.h"
#include "wrapper.h"

void initNNUE(char *file)
{
    nnue_init(file);
}

int evaluateNNUE(int turn, int *pieces, int *squares)
{
    return nnue_evaluate(turn, pieces, squares);
}

int evaluateFromFen(char *fen)
{
    return nnue_evaluate_fen(fen);
}