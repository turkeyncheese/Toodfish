#pragma once
#include "position.h"
#include "zobrist.h"
#include <stdatomic.h>

enum {
    MAX_DEPTH = 127,
    MIN_DEPTH = -8,
    MAX_PLY = MAX_DEPTH - MIN_DEPTH + 2,
};

typedef struct {
    int64_t movetime, time, inc;
    uint64_t nodes;
    int depth, movestogo;
    atomic_bool infinite; // IO thread can change this while Timer thread is checking it
} Limits;

int mated_in(int ply);
int mate_in(int ply);
bool is_mate_score(int score);

extern atomic_bool Stop; // set this to true to stop the search

extern Position rootPos;
extern ZobristStack rootStack;
extern Limits lim;
extern int Contempt;
extern const int Tempo;

void search_init(void);
uint64_t search_go(void);
void *search_posix(void *); // POSIX wrapper for pthread_create()
