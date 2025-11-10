#include "uci.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "et.h"
#include "history.h"
#include "movegen.h"
#include "perft.h"
#include "search.h"
#include "tt.h"
#include "util.h"

// The code is to allow the engine to connect to GUI's via UCI protocol

int parse_move(char *input_string, MoveList *move_list)
{
    int src = (input_string[1] - '0' - 1) * 8 + (tolower(input_string[0]) - 'a');
    int dst = (input_string[3] - '0' - 1) * 8 + (tolower(input_string[2]) - 'a');
    int promotion_piece = 0;

    for (int i = 0; i < move_list->next_open; i++) {
        Move move = move_list->move[i];
        promotion_piece = GET_MOVE_PROMOTION(move);

        if (GET_MOVE_SRC(move) == src && GET_MOVE_DST(move) == dst) {
            if (promotion_piece) {
                if (promotion_piece == Q && tolower(input_string[4]) == 'q') {
                    return i;
                }
                if (promotion_piece == N && tolower(input_string[4]) == 'n') {
                    return i;
                }
                if (promotion_piece == B && tolower(input_string[4]) == 'b') {
                    return i;
                }
                if (promotion_piece == R && tolower(input_string[4]) == 'r') {
                    return i;
                }
                continue;
            }
            return i;
        }
    }
    return -1;
}

// Example command
// position fen r2qkbnr/ppp2ppp/2np4/4N3/2B1P3/2N5/PPPP1PPP/R1BbK2R w KQkq - 0 6 moves c4f7 e8e7 c3d5
void parse_position(char *line, GameState *pos)
{
    line += 9; // Start the line after the word "position"
    char *temp = line;
    history_index = 0;
    memset(pos_history, 0, sizeof(pos_history));

    if (strncmp(line, "startpos", 8) == 0) {
        load_fen(pos, STARTING_FEN);
    }
    else if (strncmp(line, "fen", 3) == 0) {
        temp += 4; // Start temp after the word "fen"
        load_fen(pos, temp);
    }
    else {
        load_fen(pos, STARTING_FEN);
    }

    temp = strstr(line, "moves");
    if (temp != NULL) {
        int size;
        MoveList move_list;
        temp += 6; // Length of "moves "

        while (*temp) {
            size = generate_moves(pos, &move_list);
            int idx = parse_move(temp, &move_list);
            int piece = GET_MOVE_PIECE(move_list.move[idx]);

            if (idx == -1) {
                break;
            }

            GameState temp_pos;
            if (!make_move(pos, &temp_pos, move_list.move[idx]) && !size) {
                break;
            }

            // If the move is a pawn push or capture, reset history list
            if (GET_MOVE_CAPTURED(move_list.move[idx]) != NO_CAPTURE || piece == P || piece == p) {
                history_index = 0;
            }

            // Add the legal move to history
            pos_history[history_index++] = temp_pos.key;

            *pos = temp_pos;
            // Increment temp till the next move
            while (*temp && *temp != ' ') {
                temp++;
            }
            temp++;
        }
    }
}

void parse_see(char *line, const GameState *pos)
{
    line += 4; // Start the line after the word "see"
    char *temp = line;

    if (temp != NULL) {
        int src = (temp[1] - '0' - 1) * 8 + (tolower(temp[0]) - 'a');
        printf("SEE: %d\n", see(pos, src));
    }
}

void parse_go(char *line, GameState *pos)
{
    int depth = -1, movestogo = 35, movetime = -1;
    int wtime = -1, btime = -1, time = -1, inc = 0;

    SearchInfo info;
    info.stopped = false;
    info.timeset = false;

    line += 3; // Start the line after the word "go"
    char *temp = line;

    if (strncmp(line, "perft", 5) == 0) {
        info.depth = atoi(temp + 6);
        run_perft(info.depth, pos);
        return;
    }

    temp = strstr(line, "infinite");
    if (temp != NULL) {
        ;
    }

    temp = strstr(line, "winc");
    if (temp != NULL && pos->turn == WHITE) {
        inc = atoi(temp + 5);
    }

    temp = strstr(line, "binc");
    if (temp != NULL && pos->turn == BLACK) {
        inc = atoi(temp + 5);
    }


    temp = strstr(line, "wtime");
    if (temp != NULL && pos->turn == WHITE) {
        wtime = atoi(temp + 6);
        time = wtime;
    }

    temp = strstr(line, "btime");
    if (temp != NULL && pos->turn == BLACK) {
        btime = atoi(temp + 6);
        time = btime;
    }

    temp = strstr(line, "movestogo");
    if (temp != NULL) {
        movestogo = atoi(temp + 10);
    }

    temp = strstr(line, "movetime");
    if (temp != NULL) {
        movetime = atoi(temp + 9);
    }

    temp = strstr(line, "depth");
    if (temp != NULL) {
        depth = atoi(temp + 6);
    }

    if (movetime != -1) {
        time = movetime;
        movestogo = 1;
    }

    info.starttime = get_time_ms();

    if (time != -1) {
        int time_left = time;
        info.timeset = 1;
        time /= movestogo;
        time -= 50;

        if (time_left <= inc) {
            time = 0;
            inc -= 750;
            if (inc < 0)
                inc = 1;
        }
        info.stoptime = info.starttime + time + inc;
    }

    if (depth == -1) {
        info.depth = MAX_PLY;
    }
    else {
        info.depth = depth;
    }
    pos_history[history_index] = pos->key;
    search_root(pos, &info);
}

void uci_loop()
{
    GameState pos;

    // Needed to correctly output to GUI program, not sure why
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    char *buf = malloc(UCI_BUFFER_LEN * sizeof(char));
    if (buf == NULL) {
        fprintf(stderr, "uci loop: malloc failed\n");
        exit(EXIT_FAILURE);
    }
    parse_position("position startpos", &pos);
    while (1) {
        memset(buf, 0, sizeof(char) * UCI_BUFFER_LEN);
        fflush(stdout);

        if (fgets(buf, sizeof(char) * UCI_BUFFER_LEN, stdin) == NULL) {
            exit(1);
        }

        // Engine should respond with "readyok\n"
        if (strncmp(buf, "isready", 7) == 0) {
            printf("readyok\n");
            continue;
        }

        if (strncmp(buf, "position", 8) == 0) {
            parse_position(buf, &pos);
        }
        else if (strncmp(buf, "ucinewgame", 10) == 0) {
            parse_position("position startpos", &pos);
        }
        else if (strncmp(buf, "go", 2) == 0) {
            parse_go(buf, &pos);
        }
        else if (strncmp(buf, "d\n", 2) == 0) {
            print_board(pos);
        }
        else if (strncmp(buf, "see", 3) == 0) {
            parse_see(buf, &pos);
        }
        else if (strncmp(buf, "quit", 4) == 0) {
            free(buf);
            return;
        }
        else if (strncmp(buf, "setoption name Hash", 19) == 0) {
            int MB = atoi(buf + 25);
            if (MB <= 0 || MB > 1024) {
                MB = 1;
            }
            TT_SIZE = (1 << 20) * MB;
            ET_SIZE = (TT_SIZE >> 2);
            init_tt(&GLOBAL_TT);
            init_et(&GLOBAL_ET);
        }
        else if (strncmp(buf, "uci", 3) == 0) {
            // Print engine info
            printf("id name Saxton\n");
            printf("id author Aaron Lampert\n");
            //printf("option name Threads type spin default 1 min 1 max 12\n");
            printf("option name Hash type spin default 64 min 1 max 1024\n");
            printf("uciok\n");
        }
    }
}
