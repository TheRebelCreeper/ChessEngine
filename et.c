#include "et.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// Default to 16 MB
uint32_t ET_SIZE = ((1 << 20) * 16);
ET GLOBAL_ET;

void init_et(ET *table)
{
    table->num_entries = ET_SIZE / sizeof(ETEntry);
    table->num_entries -= 2;
    if (table->hash_table != NULL)
        free(table->hash_table);
    table->hash_table = (ETEntry *) malloc(table->num_entries * sizeof(ETEntry));
    if (table->hash_table == NULL) {
        perror("Failed to allocate hash table\n");
    }
    clear_et(table);
}

void clear_et(ET *table)
{
    ETEntry *e;
    if (table != NULL && table->hash_table != NULL) {
        for (e = table->hash_table; e < table->hash_table + table->num_entries; e++) {
            e->key = 0ULL;
            e->eval = INVALID_EVALUATION;
        }
        table->new_write = 0;
    }
}

// Should return a score
int probe_et(const GameState *pos)
{
    /* Cool tech - mulhi trick */
    size_t i = (size_t) (((U128) pos->key * (U128) GLOBAL_ET.num_entries) >> 64);
    int packed_key = (int) pos->key;

    ETEntry entry = GLOBAL_ET.hash_table[i];

    if (entry.key == packed_key && entry.eval != INVALID_EVALUATION) {
        return entry.eval;
    }
    return INVALID_EVALUATION;
}

void save_et(GameState *pos, int eval)
{
    /* Cool tech - mulhi trick */
    size_t i = (size_t) (((U128) pos->key * (U128) GLOBAL_ET.num_entries) >> 64);
    int packed_key = (int) pos->key;
    GLOBAL_ET.hash_table[i].key = packed_key;
    GLOBAL_ET.hash_table[i].eval = eval;
}
