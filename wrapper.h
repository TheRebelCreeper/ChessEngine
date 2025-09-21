#ifndef WRAPPER_H
#define WRAPPER_H

extern int FOUND_NETWORK;

void initNNUE(char *file);
int evaluateNNUE(int turn, int *pieces, int *squares);
int evaluateFromFen(char *fen);

#endif
