#include "workers.h"
#include "platform.h"
#include "search.h"
#include <stdlib.h>

Worker *Workers = NULL;
size_t WorkersCount = 0;

static void __attribute__((destructor)) workers_free(void) { free(Workers); }

void workers_clear(void) {
    for (size_t i = 0; i < WorkersCount; i++) {
        // Clear Workers[i] except .seed, which must be preserved
        uint64_t saveSeed = Workers[i].seed;
        Workers[i] = (Worker){.seed = saveSeed};
    }
}

void workers_prepare(size_t count) {
    if (count == WorkersCount)
        return;

    Workers = realloc(Workers, count * sizeof(Worker));
    WorkersCount = count;

    for (size_t i = 0; i < count; i++)
        Workers[i].seed = (uint64_t)system_msec() + i;

    workers_clear();
}

void workers_new_search(void) {
    for (size_t i = 0; i < WorkersCount; i++) {
        Workers[i].stack = rootStack;
        Workers[i].nodes = 0;
    }
}

uint64_t workers_nodes(void) {
    uint64_t total = 0;

    for (size_t i = 0; i < WorkersCount; i++)
        total += Workers[i].nodes;

    return total;
}
