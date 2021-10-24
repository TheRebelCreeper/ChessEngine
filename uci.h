#ifndef UCI_H
#define UCI_H
#include "position.h"
#include "list.h"

int parseMove(char *inputString, MoveList *moveList);
void parsePosition(char *line, GameState *pos);
void parseGo(char *line, GameState *pos);
void uciLoop();

#endif
