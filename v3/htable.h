#pragma once
#include "position.h"
#include <inttypes.h>
#include <stdbool.h>

enum { LBOUND, EXACT, UBOUND };

typedef struct {
    uint64_t key;
    union {
        uint64_t data;
        struct {
            int16_t score, eval;
            move_t move;
            int8_t depth;
            uint8_t bound : 2, date : 6;
        };
    };
} HashEntry;

void hash_prepare(uint64_t hashMB); // realloc + clear
void hash_clear(void);

HashEntry hash_read(uint64_t key, int ply);
void hash_write(uint64_t key, HashEntry *e, int ply);
void hash_prefetch(uint64_t key);

int hash_permille(void);

extern unsigned hashDate;
