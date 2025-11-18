#include "et.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Default to 16 MB
static uint32_t et_size = ((1 << 20) * 16);
static ET eval_table;

void init_et()
{
    eval_table.num_entries = et_size / sizeof(ETEntry);
    eval_table.num_entries -= 2;
    if (eval_table.hash_table != NULL)
        free(eval_table.hash_table);
    eval_table.hash_table = (ETEntry *) malloc(eval_table.num_entries * sizeof(ETEntry));
    if (eval_table.hash_table == NULL) {
        perror("Failed to allocate hash table\n");
    }
    clear_et();
}

void clear_et()
{
    ETEntry *e;
    if (eval_table.hash_table != NULL) {
        for (e = eval_table.hash_table; e < eval_table.hash_table + eval_table.num_entries; e++) {
            e->key = 0ULL;
            e->eval = INVALID_EVALUATION;
        }
        eval_table.new_write = 0;
    }
}

void set_et_size(int mb)
{
    et_size = (1 << 20) * mb;
}

// Should return a score
int probe_et(const GameState *pos)
{
    /* Cool tech - mulhi trick */
    size_t i = (size_t) (((U128) pos->key * (U128) eval_table.num_entries) >> 64);
    int packed_key = (int) pos->key;

    ETEntry entry = eval_table.hash_table[i];

    if (entry.key == packed_key && entry.eval != INVALID_EVALUATION) {
        return entry.eval;
    }
    return INVALID_EVALUATION;
}

void save_et(GameState *pos, int eval)
{
    /* Cool tech - mulhi trick */
    size_t i = (size_t) (((U128) pos->key * (U128) eval_table.num_entries) >> 64);
    int packed_key = (int) pos->key;
    eval_table.hash_table[i].key = packed_key;
    eval_table.hash_table[i].eval = eval;
}
