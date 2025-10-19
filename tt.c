#include "tt.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// Default memory to 128 MB
uint32_t TT_SIZE = ((1 << 20) * 128);
TT GLOBAL_TT;

void init_tt(TT *table)
{
    table->num_entries = TT_SIZE / sizeof(TTEntry);
    table->num_entries -= 2;
    if (table->hash_table != NULL)
        free(table->hash_table);
    table->hash_table = (TTEntry *) malloc(table->num_entries * sizeof(TTEntry));
    if (table->hash_table == NULL) {
        perror("Failed to allocate hash table\n");
    }
    clear_tt(table);
    printf("HashTable init complete with %d entries\n", table->num_entries);
}

void clear_tt(TT *table)
{
    if (table != NULL && table->hash_table != NULL) {
        for (TTEntry *e = table->hash_table; e < table->hash_table + table->num_entries; e++) {
            e->key = 0ULL;
            e->move = 0;
            e->depth = 0;
            e->score = INVALID_SCORE;
            e->bound = 0;
        }
        table->new_write = 0;
    }
}

// Should return a score
int probe_tt(const GameState *pos, Move *move, int alpha, int beta, int depth, int ply)
{
    int i = pos->key % GLOBAL_TT.num_entries;
    TTEntry entry = GLOBAL_TT.hash_table[i];
    if (entry.key == pos->key) {
        *move = entry.move;
        if (entry.depth >= depth) {
            GLOBAL_TT.hit++;

            int score = entry.score;
            if (score > MAX_MATE_SCORE) {
                score -= ply;
            }
            else if (score < -MAX_MATE_SCORE) {
                score += ply;
            }

            if ((entry.bound == TT_CUT && score >= beta) ||
                (entry.bound == TT_ALL && score <= alpha) ||
                entry.bound == TT_PV) {
                return score;
            }
        }
    }
    return INVALID_SCORE;
}

void save_tt(const GameState *pos, Move move, int score, int bound, int depth, int ply)
{
    int index = pos->key % GLOBAL_TT.num_entries;

    /*
    // Debug stats
    if( GLOBAL_TT.hash_table[index].key == 0)
    {
        GLOBAL_TT.new_write++;
    }
    else
    {
        GLOBAL_TT.over_write++;
    }*/

    if (score > MAX_MATE_SCORE) {
        score += ply;
    }
    else if (score < -MAX_MATE_SCORE) {
        score -= ply;
    }

    GLOBAL_TT.hash_table[index].move = move;
    GLOBAL_TT.hash_table[index].key = pos->key;
    GLOBAL_TT.hash_table[index].bound = bound;
    GLOBAL_TT.hash_table[index].score = score;
    GLOBAL_TT.hash_table[index].depth = (unsigned char) depth;
}
