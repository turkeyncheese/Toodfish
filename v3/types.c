#include "types.h"

int64_t dbgCnt[2] = {0, 0};

void eval_add(eval_t *e1, eval_t e2) {
    e1->op += e2.op;
    e1->eg += e2.eg;
}

void eval_sub(eval_t *e1, eval_t e2) {
    e1->op -= e2.op;
    e1->eg -= e2.eg;
}

bool eval_eq(eval_t e1, eval_t e2) { return e1.op == e2.op && e1.eg == e2.eg; }

int opposite(int color) {
    BOUNDS(color, NB_COLOR);
    return color ^ BLACK; // branchless for: color == WHITE ? BLACK : WHITE
}

int push_inc(int color) {
    BOUNDS(color, NB_COLOR);
    return UP - color * (UP - DOWN); // branchless for: color == WHITE ? UP : DOWN
}

int square_from(int rank, int file) {
    BOUNDS(rank, NB_RANK);
    BOUNDS(file, NB_FILE);
    return NB_FILE * rank + file;
}

int rank_of(int square) {
    BOUNDS(square, NB_SQUARE);
    return square / NB_FILE;
}

int file_of(int square) {
    BOUNDS(square, NB_SQUARE);
    return square % NB_FILE;
}

int relative_rank(int color, int rank) {
    BOUNDS(color, NB_COLOR);
    BOUNDS(rank, NB_RANK);
    return rank ^ (RANK_8 * color); // branchless for: color == WHITE ? rank : RANK_8 - rank
}

int relative_rank_of(int color, int square) {
    BOUNDS(square, NB_SQUARE);
    return relative_rank(color, rank_of(square));
}

int move_from(move_t m) { return m & 077; }

int move_to(move_t m) { return (m >> 6) & 077; }

int move_prom(move_t m) {
    const unsigned prom = m >> 12;
    assert(prom <= QUEEN || prom == NB_PIECE);
    return (int)prom;
}

move_t move_build(int from, int to, int prom) {
    BOUNDS(from, NB_SQUARE);
    BOUNDS(to, NB_SQUARE);
    assert((unsigned)prom <= QUEEN || prom == NB_PIECE);
    return (move_t)(from | (to << 6) | (prom << 12));
}
