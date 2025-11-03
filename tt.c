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
            e->flag = TT_NONE;
        }
        table->new_write = 0;
    }
}

// Should return a score
bool probe_tt(const GameState *pos, TTEntry *dst, int ply)
{
    /* Cool tech - mulhi trick
    size_t idx = (size_t)((U128)key * (U128)size) >> 64);
    */
    int i = pos->key % GLOBAL_TT.num_entries;
    TTEntry entry = GLOBAL_TT.hash_table[i];

    if (entry.key == pos->key && entry.flag != TT_NONE) {
        // Adjust mate score in TT
        int score = entry.score;
        if (score > MAX_MATE_SCORE) {
            score -= ply;
        }
        else if (score < -MAX_MATE_SCORE) {
            score += ply;
        }

        dst->score = score;
        dst->depth = entry.depth;
        dst->move = entry.move;
        dst->flag = entry.flag;
        GLOBAL_TT.hit++;

        return true;
    }
    dst->flag = TT_NONE;
    dst->move = 0;
    return false;
}

void save_tt(const GameState *pos, Move move, int score, int flag, int depth, int ply)
{
    assert(depth >= 0);
    int i = pos->key % GLOBAL_TT.num_entries;

    if (score > MAX_MATE_SCORE) {
        score += ply;
    }
    else if (score < -MAX_MATE_SCORE) {
        score -= ply;
    }

    if (move || GLOBAL_TT.hash_table[i].key != pos->key) {
        GLOBAL_TT.hash_table[i].move = move;
    }

    GLOBAL_TT.hash_table[i].key = pos->key;
    GLOBAL_TT.hash_table[i].flag = flag;
    GLOBAL_TT.hash_table[i].score = score;
    GLOBAL_TT.hash_table[i].depth = (unsigned char) depth;
}
