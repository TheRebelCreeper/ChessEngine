#include "perft.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

U64 perft(int depth, GameState *pos)
{
    MoveList move_list;
    int size, legal;
    U64 sum = 0;
    if (depth == 0) {
        return 1ULL;
    }

    size = generate_moves(pos, &move_list);

    for (int i = 0; i < size; i++) {
        GameState new_pos = play_move(pos, move_list.move[i], &legal);
        if (legal) {
            sum += perft(depth - 1, &new_pos);
        }
    }
    return sum;
}

U64 perft_divide(int depth, GameState *pos)
{
    MoveList move_list;
    int size, i, legal;
    U64 sum = 0;

    if (depth == 0) {
        return 1ULL;
    }

    size = generate_moves(pos, &move_list);
    printf("Perft results for depth %d:\n", depth);

    for (i = 0; i < size; i++) {
        Move current = move_list.move[i];
        GameState new_pos = play_move(pos, current, &legal);
        if (legal == 1) {
            U64 res = perft(depth - 1, &new_pos);
            sum += res;
            print_move(current);
            printf(": %llu\n", res);
        }
    }
    return sum;
}

U64 run_perft(int depth, GameState *pos)
{
    unsigned int start = get_time_ms();
    U64 size = perft_divide(depth, pos);
    printf("Nodes searched: %llu\n\n", size);
    unsigned int finish = get_time_ms();
    double seconds = (finish - start) / 1000.0;
    printf("Finished perft in %f seconds\n", seconds);
    printf("NPS: %f\n", size / seconds);
    return size;
}

void parse_perft_line(char *line, GameState *pos)
{
    if (line == NULL)
        exit(EXIT_FAILURE);

    int length = strlen(line);
    char *str = malloc(length + 1);
    if (str == NULL) {
        perror("Malloc error in loadFEN");
        exit(EXIT_FAILURE);
    }
    memset(str, 0, length + 1);
    memcpy(str, line, length);
    str[length] = 0;

    load_fen(pos, str);
    *strchr(line, ';') = 0;

    while (*str) {
        while (*str != ';' || *str == ' ') {
            if (*str == 0 || *str == '\n')
                return;
            str++;
        }
        str += 2;
        int depth = str[0] - '0';
        str += 2;
        U64 expected = strtoll(str, NULL, 10);
        printf("Depth: %d | FEN: %s | Expected: %llu\n", depth, line, expected);
        U64 found = perft(depth, pos);
        if (found != expected) {
            fprintf(stderr, "PERFT FAILURE. Found %llu\n", found);
            exit(1);
        }
    }
    free(str);
}

void parse_epd_file(const char *filename, GameState *pos)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[2048];
    while (fgets(line, sizeof(line), fp) != NULL) {
        parse_perft_line(line, pos);
    }
}

void moveGeneratorValidator()
{
    GameState testPos;
    parse_epd_file("standard.epd", &testPos);
    printf("PERFT PASSED\n");
}
