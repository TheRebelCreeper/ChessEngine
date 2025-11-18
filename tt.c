#include "tt.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Default memory to 128 MB
static uint32_t tt_size = ((1 << 20) * 128);
static TT transposition_table;

void init_tt()
{
    transposition_table.num_entries = tt_size / sizeof(TTEntry);
    transposition_table.num_entries -= 2;
    if (transposition_table.hash_table != NULL)
        free(transposition_table.hash_table);
    transposition_table.hash_table = (TTEntry *) malloc(transposition_table.num_entries * sizeof(TTEntry));
    if (transposition_table.hash_table == NULL) {
        perror("Failed to allocate hash table\n");
    }
    clear_tt();
}

void clear_tt()
{
    if (transposition_table.hash_table != NULL) {
        for (TTEntry *e = transposition_table.hash_table;
             e < transposition_table.hash_table + transposition_table.num_entries; e++) {
            e->key = 0ULL;
            e->move = 0;
            e->depth = 0;
            e->score = INVALID_SCORE;
            e->flag = TT_NONE;
        }
        transposition_table.new_write = 0;
    }
}

void set_tt_size(int mb)
{
    tt_size = (1 << 20) * mb;
}

// Should return a score
bool probe_tt(const GameState *pos, TTEntry *dst, int ply)
{
    /* Cool tech - mulhi trick */
    size_t i = (size_t) (((U128) pos->key * (U128) transposition_table.num_entries) >> 64);
    int packed_key = (int) pos->key;

    TTEntry entry = transposition_table.hash_table[i];

    if (entry.key == packed_key && entry.flag != TT_NONE) {
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
        transposition_table.hit++;

        return true;
    }
    dst->flag = TT_NONE;
    dst->move = 0;
    return false;
}

void save_tt(const GameState *pos, Move move, int score, int flag, int depth, int ply)
{
    assert(depth >= 0);

    /* Cool tech - mulhi trick */
    size_t i = (size_t) (((U128) pos->key * (U128) transposition_table.num_entries) >> 64);
    int packed_key = (int) pos->key;

    if (score > MAX_MATE_SCORE) {
        score += ply;
    }
    else if (score < -MAX_MATE_SCORE) {
        score -= ply;
    }

    if (move || transposition_table.hash_table[i].key != packed_key) {
        transposition_table.hash_table[i].move = move;
    }

    transposition_table.hash_table[i].key = packed_key;
    transposition_table.hash_table[i].flag = flag;
    transposition_table.hash_table[i].score = score;
    transposition_table.hash_table[i].depth = (unsigned char) depth;
}
