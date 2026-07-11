#include "wrapper.h"
#include "nnue.h"

int FOUND_NETWORK = 0;

void init_nnue(char *file)
{
    FOUND_NETWORK = nnue_init(file);
}

int evaluate_nnue(int turn, int *pieces, int *squares)
{
    return nnue_evaluate(turn, pieces, squares);
}

int evaluate_nnue_incremental(int turn, int *pieces, int *squares, NNUEdata **nnue_data)
{
    return nnue_evaluate_incremental(turn, pieces, squares, nnue_data);
}

int evaluate_from_fen(char *fen)
{
    return nnue_evaluate_fen(fen);
}
