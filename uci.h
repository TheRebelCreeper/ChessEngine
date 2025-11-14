#ifndef UCI_H
#define UCI_H
#include "list.h"
#include "position.h"

#define UCI_BUFFER_LEN 8192

int parse_move(char *input_string, MoveList *move_list);
void parse_position(char *line, GameState *pos);
void parse_go(char *line, GameState *pos);
void uci_loop();

#endif
