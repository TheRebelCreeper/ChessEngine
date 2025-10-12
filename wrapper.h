#ifndef WRAPPER_H
#define WRAPPER_H

extern int FOUND_NETWORK;

void init_nnue(char *file);
int evaluate_nnue(int turn, int *pieces, int *squares);
int evaluate_from_fen(char *fen);

#endif
