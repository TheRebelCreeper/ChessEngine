#ifndef UCI_H
#define UCI_H
#include "list.h"
#include "position.h"

#define UCI_BUFFER_LEN 8192

int parseMove(char *inputString, MoveList *moveList);
void parsePosition(char *line, GameState *pos);
void parseGo(char *line, GameState *pos);
void uciLoop();

#endif
