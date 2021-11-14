#include "nnue.h"
#include "wrapper.h"

int FOUND_NETWORK;

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