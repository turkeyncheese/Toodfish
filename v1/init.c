#include "defs.h"
#include <stdlib.h>

#define RAND_64 ((U64) rand() | \
                 (U64) rand() << 15 | \
                 (U64) rand() << 30 | \
                 (U64) rand() << 55 | \
                 ((U64) rand() & 0xf) << 60)

int sq120ToSq64[BRD_SQ_NUM];
int sq64ToSq120[64];

U64 setMask[64];
U64 clearMask[64];

U64 pieceKeys[13][120];
U64 sideKey;
U64 castleKeys[16];

int filesBrd[BRD_SQ_NUM];
int ranksBrd[BRD_SQ_NUM];

U64 fileBBMask[8];
U64 rankBBMask[8];

U64 blackPassedMask[64];
U64 whitePassedMask[64];
U64 isolatedMask[64];

S_OPTIONS engineOptions[1];

void initEvalMasks() {

	int sq, toSq;

	for (int sq = 0; sq < 8; ++sq) {
        fileBBMask[sq] = 0ULL;
		rankBBMask[sq] = 0ULL;
	}

	for (int r = RANK_8; r >= RANK_1; r--) {
        for (int f = FILE_A; f <= FILE_H; f++) {
            sq = r * 8 + f;
            fileBBMask[f] |= (1ULL << sq);
            rankBBMask[r] |= (1ULL << sq);
        }
	}

	for (int sq = 0; sq < 64; ++sq) {
		isolatedMask[sq] = 0ULL;
		whitePassedMask[sq] = 0ULL;
		blackPassedMask[sq] = 0ULL;
    }

	for (int sq = 0; sq < 64; ++sq) {
		toSq = sq + 8;

        while (toSq < 64) {
            whitePassedMask[sq] |= (1ULL << toSq);
            toSq += 8;
        }

        toSq = sq - 8;
        while (toSq >= 0) {
            blackPassedMask[sq] |= (1ULL << toSq);
            toSq -= 8;
        }

        if (filesBrd[SQ120(sq)] > FILE_A) {
            isolatedMask[sq] |= fileBBMask[filesBrd[SQ120(sq)] - 1];

            toSq = sq + 7;
            while (toSq < 64) {
                whitePassedMask[sq] |= (1ULL << toSq);
                toSq += 8;
            }

            toSq = sq - 9;
            while (toSq >= 0) {
                blackPassedMask[sq] |= (1ULL << toSq);
                toSq -= 8;
            }
        }

        if (filesBrd[SQ120(sq)] < FILE_H) {
            isolatedMask[sq] |= fileBBMask[filesBrd[SQ120(sq)] + 1];

            toSq = sq + 9;
            while (toSq < 64) {
                whitePassedMask[sq] |= (1ULL << toSq);
                toSq += 8;
            }

            toSq = sq - 7;
            while (toSq >= 0) {
                blackPassedMask[sq] |= (1ULL << toSq);
                toSq -= 8;
            }
        }
	}
}

void initFilesRanksBrd() {
    int sq;

    for (int i = 0; i < BRD_SQ_NUM; i++) {
        filesBrd[i] = OFFBOARD;
        ranksBrd[i] = OFFBOARD;
    }

    for (int rank = RANK_1; rank <= RANK_8; rank++) {
        for (int file = FILE_A; file <= FILE_H; file++) {
            sq = FR2SQ(file, rank);
            filesBrd[sq] = file;
            ranksBrd[sq] = rank;
        }
    }
}

void initHashKeys() {
    for (int i = 0; i < 13; i++) {
        for (int j = 0; j < 120; j++) {
            pieceKeys[i][j] = RAND_64;
        }
    }

    sideKey = RAND_64;

    for (int i = 0; i < 16; i++) {
        castleKeys[i] = RAND_64;
    }
}

void initBitMasks() {
    for (int i = 0; i < 64; i++) {
        setMask[i] = 0ULL;
        clearMask[i] = 0ULL;
    }

    for (int i = 0; i < 64; i++) {
        setMask[i] |= (1ULL << i);
        clearMask[i] = ~setMask[i];
    }
}

void initoSq120To64() {
    int file = FILE_A;
    int rank = RANK_1;
    int sq = A1;
    int sq64 = 0;

    for (int i = 0; i < BRD_SQ_NUM; i++) {
        sq120ToSq64[i] = 65;
    }

    for (int i = 0; i < 64; i++) {
        sq64ToSq120[i] = 120;
    }

    for (rank = RANK_1; rank <= RANK_8; rank++) {
        for (file = FILE_A; file <= FILE_H; file++) {
            sq = FR2SQ(file, rank);

            sq64ToSq120[sq64] = sq;
            sq120ToSq64[sq] = sq64;

            sq64++;
        }
    }
}

void initAll() {
    initoSq120To64();
    initBitMasks();
    initHashKeys();
    initFilesRanksBrd();
    initEvalMasks();
    initMvvLva();
}