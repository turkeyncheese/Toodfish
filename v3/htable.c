#include "htable.h"
#include "platform.h"
#include "search.h"
#include <stdlib.h>
#include <string.h>

static HashEntry *HashTable = NULL;
static size_t HashCount = 0;

unsigned hashDate = 0;

static int score_to_hash(int score, int ply) {
    if (score >= mate_in(MAX_PLY))
        score += ply;
    else if (score <= mated_in(MAX_PLY))
        score -= ply;

    assert(abs(score) < MATE);

    return score;
}

static int score_from_hash(int hashScore, int ply) {
    if (hashScore >= mate_in(MAX_PLY))
        hashScore -= ply;
    else if (hashScore <= mated_in(MAX_PLY))
        hashScore += ply;

    assert(abs(hashScore) < MATE - ply);

    return hashScore;
}

static __attribute__((destructor)) void hash_free(void) { free(HashTable); }

void hash_prepare(uint64_t hashMB) {
    assert(bb_count(hashMB) == 1); // must be a power of 2

    HashCount = (hashMB << 20) / sizeof(HashEntry);
    HashTable = realloc(HashTable, HashCount * sizeof(HashEntry));

    // All 64-bit malloc() implementations should return 16-byte aligned memory. We want this for
    // performance, to ensure that no HashEntry sits across two cache lines.
    assert((uintptr_t)HashTable % sizeof(HashEntry) == 0);

    hash_clear();
}

void hash_clear(void) {
    memset(HashTable, 0, HashCount * sizeof(HashEntry));
    hashDate = 0;
}

HashEntry hash_read(uint64_t key, int ply) {
    HashEntry e = HashTable[key & (HashCount - 1)];

    if (e.key == key)
        e.score = (int16_t)score_from_hash(e.score, ply);
    else
        e.data = 0;

    return e;
}

void hash_write(uint64_t key, HashEntry *e, int ply) {
    HashEntry *slot = &HashTable[key & (HashCount - 1)];

    e->date = (uint8_t)hashDate;
    assert(e->date == hashDate % 64);

    if (e->date != slot->date || e->depth >= slot->depth) {
        e->score = (int16_t)score_to_hash(e->score, ply);
        e->key = key;
        *slot = *e;
    }
}

void hash_prefetch(uint64_t key) { __builtin_prefetch(&HashTable[key & (HashCount - 1)]); }

int hash_permille(void) {
    int result = 0;

    for (int i = 0; i < 1000; i++)
        result += HashTable[i].key && HashTable[i].date == hashDate % 64;

    return result;
}
