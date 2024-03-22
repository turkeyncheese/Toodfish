#pragma once
#include "platform.h"
#include "position.h"

#define NB_LEVEL 14

void uci_loop(void);

typedef struct {
    mtx_t mtx;
    int64_t start;
    double variability;
    int lastDepth;
    move_t best, ponder;
} Info;

extern Info ui;
extern int uciLevel;
extern int64_t uciTimeBuffer;
extern bool uciChess960, uciFakeTime;
extern size_t uciHash, uciThreads;

void info_create(Info *info);
void info_destroy(Info *info);

void info_update(Info *info, int depth, int score, uint64_t nodes, move_t pv[], bool partial);
void info_print_bestmove(Info *info);
move_t info_best(Info *info);
int info_last_depth(Info *info);
double info_variability(Info *info);
