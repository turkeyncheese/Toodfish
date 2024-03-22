#include <stdio.h>
#include <stdbool.h>
#include "defs.h"

const int pawnIsolated = -10;
const int pawnPassed[8] = {0, 5, 10, 20, 35, 60, 100, 200}; 
const int rookOpenFile = 10;
const int rookSemiOpenFile = 5;
const int queenOpenFile = 5;
const int queenSemiOpenFile = 3;
const int bishopPair = 30;

const int pawnTable[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    10, 10, 0, -10, -10, 0, 10, 10,
    5, 0, 0, 5, 5, 0, 0, 5,
    0, 0, 10, 20, 20, 10, 0, 0,
    5, 5, 5, 10, 10, 5, 5, 5,
    10, 10, 10, 20, 20, 10, 10, 10,
    20, 20, 20, 30, 30, 20, 20, 20,
    0, 0, 0, 0, 0, 0, 0, 0 
};

const int knightTable[64] = {
    0, -10, 0, 0, 0, 0, -10, 0,
    0, 0, 0, 5, 5, 0, 0, 0,
    0, 0, 10, 10, 10, 10, 0, 0,
    0, 0, 10, 20, 20, 10, 5, 0,
    5, 10, 15, 20, 20, 15, 10, 5,
    5, 10, 10, 20, 20, 10, 10, 5,
    0, 0, 5, 10, 10, 5, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0 
};

const int bishopTable[64] = {
    0, 0, -10, 0, 0, -10, 0, 0,
    0, 0, 0, 10, 10, 0, 0, 0,
    0, 0, 10, 15, 15, 10, 0, 0,
    0, 10, 15, 20, 20, 15, 10, 0,
    0, 10, 15, 20, 20, 15, 10, 0,
    0, 0, 10, 15, 15, 10, 0, 0,
    0, 0, 0, 10, 10, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0 
};

const int rookTable[64] = {
    0, 0, 5, 10, 10, 5, 0, 0,
    0, 0, 5, 10, 10, 5, 0, 0,
    0, 0, 5, 10, 10, 5, 0, 0,
    0, 0, 5, 10, 10, 5, 0, 0,
    0, 0, 5, 10, 10, 5, 0, 0,
    0, 0, 5, 10, 10, 5, 0, 0,
    25, 25, 25, 25, 25, 25, 25, 25,
    0, 0, 5, 10, 10, 5, 0, 0 
};

const int kingE[64] = { 
    -50, -10, 0, 0, 0, 0, -10, -50,
    -10, 0, 10, 10, 10, 10, 0, -10,
    0, 10, 20, 20, 20, 20, 10, 0,
    0, 10, 20, 40, 40, 20, 10, 0,
    0, 10, 20, 40, 40, 20, 10, 0,
    0, 10, 20, 20, 20, 20, 10, 0,
    -10, 0, 10, 10, 10, 10, 0, -10,
    -50, -10, 0, 0, 0, 0, -10, -50 
};

const int kingO[64] = { 
    0, 5, 5, -10, -10, 0, 10, 5,
    -30, -30, -30, -30, -30, -30, -30, -30,
    -50, -50, -50, -50, -50, -50, -50, -50,
    -70, -70, -70, -70, -70, -70, -70, -70,
    -70, -70, -70, -70, -70, -70, -70, -70,
    -70, -70, -70, -70, -70, -70, -70, -70,
    -70, -70, -70, -70, -70, -70, -70, -70,
    -70, -70, -70, -70, -70, -70, -70, -70 
};

int materialDraw(const S_BOARD *pos) {

	ASSERT(checkBoard(pos));
	
    if (!pos -> pieceNum[wR] && !pos -> pieceNum[bR] && !pos -> pieceNum[wQ] && !pos -> pieceNum[bQ]) {
	    if (!pos -> pieceNum[bB] && !pos -> pieceNum[wB]) {
	        if (pos -> pieceNum[wN] < 3 && pos -> pieceNum[bN] < 3) {return true;}
	    }
        
        else if (!pos -> pieceNum[wN] && !pos -> pieceNum[bN]) {
	        if (abs(pos -> pieceNum[wB] - pos -> pieceNum[bB]) < 2) {return true;}
	    }
        
        else if ((pos -> pieceNum[wN] < 3 && !pos -> pieceNum[wB]) || (pos -> pieceNum[wB] == 1 && !pos -> pieceNum[wN])) {
	        if ((pos -> pieceNum[bN] < 3 && !pos -> pieceNum[bB]) || (pos -> pieceNum[bB] == 1 && !pos -> pieceNum[bN]))  {return true;}
	    }
	}
    
    else if (!pos -> pieceNum[wQ] && !pos -> pieceNum[bQ]) {
        if (pos -> pieceNum[wR] == 1 && pos -> pieceNum[bR] == 1) {
            if ((pos -> pieceNum[wN] + pos -> pieceNum[wB]) < 2 && (pos -> pieceNum[bN] + pos -> pieceNum[bB]) < 2)	{return true;}
        }
        
        else if (pos -> pieceNum[wR] == 1 && !pos -> pieceNum[bR]) {
            if ((pos -> pieceNum[wN] + pos -> pieceNum[wB] == 0) && (((pos -> pieceNum[bN] + pos -> pieceNum[bB]) == 1) || ((pos -> pieceNum[bN] + pos -> pieceNum[bB]) == 2))) {return true;}
        }
        
        else if (pos -> pieceNum[bR] == 1 && !pos -> pieceNum[wR]) {
            if ((pos -> pieceNum[bN] + pos -> pieceNum[bB] == 0) && (((pos -> pieceNum[wN] + pos -> pieceNum[wB]) == 1) || ((pos -> pieceNum[wN] + pos -> pieceNum[wB]) == 2))) {return true;}
        }
    }

    return false;
}

#define ENDGAME_MAT (1 * pieceValue[wR] + 2 * pieceValue[wN] + 2 * pieceValue[wP] + pieceValue[wK])

int evalPosition(const S_BOARD *pos) {
    ASSERT(checkBoard(pos));

    int piece;
    int sq;
    int score = pos -> material[WHITE] - pos -> material[BLACK];

    if (!pos -> pieceNum[wP] && !pos -> pieceNum[bP] && materialDraw(pos) == true) {
		return 0;
	}

    piece = wP;
    for (int pieceNum = 0; pieceNum < pos -> pieceNum[piece]; pieceNum++) {
        sq = pos -> pieceList[piece][pieceNum];
		ASSERT(sqOnBoard(sq));
		ASSERT(SQ64(sq) >= 0 && SQ64(sq) <= 63);
		score += pawnTable[SQ64(sq)];

        if ((isolatedMask[SQ64(sq)] & pos -> pawns[WHITE]) == 0) {
			score += pawnIsolated;
		}
		
		if ((whitePassedMask[SQ64(sq)] & pos -> pawns[BLACK]) == 0) {
			score += pawnPassed[ranksBrd[sq]];
		}
    }

    piece = bP;
    for (int pieceNum = 0; pieceNum < pos -> pieceNum[piece]; pieceNum++) {
        sq = pos -> pieceList[piece][pieceNum];
		ASSERT(sqOnBoard(sq));
		ASSERT(SQ64(sq) >= 0 && SQ64(sq) <= 63);
		score -= pawnTable[MIRROR64(SQ64(sq))];

        if ((isolatedMask[SQ64(sq)] & pos -> pawns[BLACK]) == 0) {
			score -= pawnIsolated;
		}
		
		if ((blackPassedMask[SQ64(sq)] & pos -> pawns[WHITE]) == 0) {
			score -= pawnPassed[7 - ranksBrd[sq]];
		}
    }

    piece = wN;
    for (int pieceNum = 0; pieceNum < pos -> pieceNum[piece]; pieceNum++) {
        sq = pos -> pieceList[piece][pieceNum];
		ASSERT(sqOnBoard(sq));
		ASSERT(SQ64(sq) >= 0 && SQ64(sq) <= 63);
		score += knightTable[SQ64(sq)];	
    }

    piece = bN;
    for (int pieceNum = 0; pieceNum < pos -> pieceNum[piece]; pieceNum++) {
        sq = pos -> pieceList[piece][pieceNum];
		ASSERT(sqOnBoard(sq));
		ASSERT(SQ64(sq) >= 0 && SQ64(sq) <= 63);
		score -= knightTable[MIRROR64(SQ64(sq))];	
    }

    piece = wB;
    for (int pieceNum = 0; pieceNum < pos -> pieceNum[piece]; pieceNum++) {
        sq = pos -> pieceList[piece][pieceNum];
		ASSERT(sqOnBoard(sq));
		ASSERT(SQ64(sq) >= 0 && SQ64(sq) <= 63);
		score += bishopTable[SQ64(sq)];	
    }

    piece = bB;
    for (int pieceNum = 0; pieceNum < pos -> pieceNum[piece]; pieceNum++) {
        sq = pos -> pieceList[piece][pieceNum];
		ASSERT(sqOnBoard(sq));
		ASSERT(SQ64(sq) >= 0 && SQ64(sq) <= 63);
		score -= bishopTable[MIRROR64(SQ64(sq))];	
    }

    piece = wR;
    for (int pieceNum = 0; pieceNum < pos -> pieceNum[piece]; pieceNum++) {
        sq = pos -> pieceList[piece][pieceNum];
		ASSERT(sqOnBoard(sq));
		ASSERT(SQ64(sq) >= 0 && SQ64(sq) <= 63);
		score += rookTable[SQ64(sq)];

        ASSERT(fileRankValid(FilesBrd[sq]));
		
		if (!(pos -> pawns[BOTH] & fileBBMask[filesBrd[sq]])) {
			score += rookOpenFile;
		}
        
        else if (!(pos -> pawns[WHITE] & fileBBMask[filesBrd[sq]])) {
			score += rookSemiOpenFile;
		}
    }

    piece = bR;
    for (int pieceNum = 0; pieceNum < pos -> pieceNum[piece]; pieceNum++) {
        sq = pos -> pieceList[piece][pieceNum];
		ASSERT(sqOnBoard(sq));
		ASSERT(SQ64(sq) >= 0 && SQ64(sq) <= 63);
		score -= rookTable[MIRROR64(SQ64(sq))];

        ASSERT(fileRankValid(FilesBrd[sq]));
		
		if (!(pos -> pawns[BOTH] & fileBBMask[filesBrd[sq]])) {
			score -= rookOpenFile;
		}
        
        else if (!(pos -> pawns[BLACK] & fileBBMask[filesBrd[sq]])) {
			score -= rookSemiOpenFile;
		}
    }

    piece = wQ;	
	for(int pieceNum = 0; pieceNum < pos -> pieceNum[piece]; pieceNum++) {
		sq = pos -> pieceList[piece][pieceNum];
		ASSERT(sqOnBoard(sq));
		ASSERT(SQ64(sq) >= 0 && SQ64(sq) <= 63);

		ASSERT(fileRankValid(filesBrd[sq]));

		if (!(pos->pawns[BOTH] & fileBBMask[filesBrd[sq]])) {
			score += queenOpenFile;
		}
        
        else if (!(pos -> pawns[WHITE] & fileBBMask[filesBrd[sq]])) {
			score += queenSemiOpenFile;
		}
	}

    piece = bQ;	
	for(int pieceNum = 0; pieceNum < pos -> pieceNum[piece]; pieceNum++) {
		sq = pos -> pieceList[piece][pieceNum];
		ASSERT(sqOnBoard(sq));
		ASSERT(SQ64(sq) >= 0 && SQ64(sq) <= 63);

		ASSERT(fileRankValid(filesBrd[sq]));

		if (!(pos->pawns[BOTH] & fileBBMask[filesBrd[sq]])) {
			score -= queenOpenFile;
		}
        
        else if (!(pos -> pawns[WHITE] & fileBBMask[filesBrd[sq]])) {
			score -= queenSemiOpenFile;
		}
	}

    piece = wK;
	sq = pos -> pieceList[piece][0];
	ASSERT(sqOnBoard(sq));
	ASSERT(SQ64(sq) >= 0 && SQ64(sq) <= 63);
	
	if ((pos -> material[BLACK] <= ENDGAME_MAT)) {
		score += kingE[SQ64(sq)];
	} else {
		score += kingO[SQ64(sq)];
	}
	
	piece = bK;
	sq = pos -> pieceList[piece][0];
	ASSERT(sqOnBoard(sq));
	ASSERT(MIRROR64(SQ64(sq)) >= 0 && MIRROR64(SQ64(sq)) <= 63);
	
	if ((pos -> material[WHITE] <= ENDGAME_MAT) ) {
		score -= kingE[MIRROR64(SQ64(sq))];
	} else {
		score -= kingO[MIRROR64(SQ64(sq))];
	}
	
	if (pos -> pieceNum[wB] >= 2) score += bishopPair;
	if (pos -> pieceNum[bB] >= 2) score -= bishopPair;

    if (pos -> side == WHITE) {
		return score;
	} else {
		return -score;
	}
}