#include "defs.h"

U64 generatePosKey(const S_BOARD *pos) {
    int piece = EMPTY;

    U64 finalKey = 0;

    for (int sq = 0; sq < BRD_SQ_NUM; sq++) {
        piece = pos -> pieces[sq];

        if (piece != NO_SQ && piece != EMPTY) {
            ASSERT(piece >= wP && piece <= bK);
            finalKey ^= pieceKeys[piece][sq];
        }
    }

    if (pos -> side == WHITE) {
        finalKey ^= sideKey;
    }

    if (pos -> enPass != NO_SQ) {
        ASSERT(pos -> enPass >= 0 && pos -> enPass < BRD_SQ_NUM);
        ASSERT(sqOnBoard(pos -> enPass));
		ASSERT(ranksBrd[pos -> enPass] == RANK_3 || ranksBrd[pos -> enPass] == RANK_6);
        finalKey ^= pieceKeys[EMPTY][pos -> enPass];
    }

    ASSERT(pos -> castlePerm >= 0 && pos -> castlePerm <= 15);
    finalKey ^= castleKeys[pos -> castlePerm];

    return finalKey;
}