#include <stdio.h>
#include <stdbool.h>
#include "defs.h"

#define HASH_PCE(piece, sq) (pos -> posKey ^= pieceKeys[piece][sq])
#define HASH_CA (pos -> posKey ^= castleKeys[pos -> castlePerm])
#define HASH_SIDE (pos -> posKey ^= sideKey)
#define HASH_EP (pos -> posKey ^= pieceKeys[EMPTY][pos -> enPass])

const int castlePerm[120] = {
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 13, 15, 15, 15, 12, 15, 15, 14, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15,  7, 15, 15, 15,  3, 15, 15, 11, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15
};

static void clearPiece(const int sq, S_BOARD *pos) {
    ASSERT(sqOnBoard(sq));
	ASSERT(checkBoard(pos));
	
    int piece = pos -> pieces[sq];
	
    ASSERT(pieceValid(pce));
	
	int color = pieceColor[piece];
	int index = 0;
	int tempPieceNum = -1;
	
	ASSERT(sideValid(color));
	
    HASH_PCE(piece, sq);
	
	pos -> pieces[sq] = EMPTY;
    pos -> material[color] -= pieceValue[piece];

    if (pieceBig[piece]) {
		pos -> bigPieces[color]--;
		
        if (pieceMajor[piece]) {
			pos -> majorPieces[color]--;
		} else {
			pos -> minorPieces[color]--;
		}
	} else {
		CLEARBIT(pos -> pawns[color], SQ64(sq));
		CLEARBIT(pos -> pawns[BOTH], SQ64(sq));
	}
	
	for (int i = 0; i < pos -> pieceNum[piece]; i++) {
		if (pos -> pieceList[piece][i] == sq) {
			tempPieceNum = i;
			break;
		}
	}
	
	ASSERT(tempPieceNum != -1);
	ASSERT(tempPieceNum >= 0 && tempPieceNum < 10);
	
	pos -> pieceNum[piece]--;		
	
	pos -> pieceList[piece][tempPieceNum] = pos -> pieceList[piece][pos -> pieceNum[piece]];
}

static void addPiece(const int sq, S_BOARD *pos, const int piece) {
    ASSERT(pieceValid(piece));
    ASSERT(sqOnBoard(sq));
	
	int color = pieceColor[piece];
	ASSERT(sideValid(color));

    HASH_PCE(piece, sq);
	
	pos -> pieces[sq] = piece;

    if (pieceBig[piece]) {
		pos -> bigPieces[color]++;
		
        if (pieceMajor[piece]) {
			pos -> majorPieces[color]++;
		} else {
			pos -> minorPieces[color]++;
		}
	} else {
		SETBIT(pos -> pawns[color], SQ64(sq));
		SETBIT(pos -> pawns[BOTH], SQ64(sq));
	}
	
	pos -> material[color] += pieceValue[piece];
	pos -> pieceList[piece][pos -> pieceNum[piece]++] = sq;
}

static void movePiece(const int from, const int to, S_BOARD *pos) {
    ASSERT(sqOnBoard(from));
    ASSERT(sqOnBoard(to));
	
	int piece = pos -> pieces[from];	
	int color = pieceColor[piece];
	ASSERT(sideValid(color));
    ASSERT(pieceValid(piece));
	
#ifdef DEBUG
	int tempPieceNum = false;
#endif

	HASH_PCE(piece, from);
	pos -> pieces[from] = EMPTY;
	
	HASH_PCE(piece, to);
	pos -> pieces[to] = piece;
	
	if (!pieceBig[piece]) {
		CLEARBIT(pos -> pawns[color], SQ64(from));
		CLEARBIT(pos -> pawns[BOTH], SQ64(from));
		SETBIT(pos -> pawns[color], SQ64(to));
		SETBIT(pos -> pawns[BOTH], SQ64(to));		
	}    
	
	for (int i = 0; i < pos -> pieceNum[piece]; i++) {
		if (pos -> pieceList[piece][i] == from) {
			pos -> pieceList[piece][i] = to;

#ifdef DEBUG
			tempPieceNum = true;
#endif
			break;
		}
	}

	ASSERT(tempPieceNum);
}

int makeMove(S_BOARD *pos, int move) {
	ASSERT(checkBoard(pos));
	
	int from = FROMSQ(move);
    int to = TOSQ(move);
    int side = pos -> side;
	
	ASSERT(sqOnBoard(from));
    ASSERT(sqOnBoard(to));
    ASSERT(sideValid(side));
    ASSERT(pieceValid(pos -> pieces[from]));
	ASSERT(pos -> hisPly >= 0 && pos -> hisPly < MAXGAMEMOVES);
	ASSERT(pos -> ply >= 0 && pos -> ply < MAXDEPTH);
	
	pos -> history[pos -> hisPly].posKey = pos -> posKey;
	
	if (move & MFLAGEP) {
        if (side == WHITE) {
            clearPiece(to - 10,pos);
        } else {
            clearPiece(to + 10,pos);
        }
    }
    
    else if (move & MFLAGCA) {
        switch (to) {
            case C1: movePiece(A1, D1, pos); break;
            case C8: movePiece(A8, D8, pos); break;
            case G1: movePiece(H1, F1, pos); break;
            case G8: movePiece(H8, F8, pos); break;
            default: ASSERT(false); break;
        }
    }	
	
	if (pos -> enPass != NO_SQ) HASH_EP;
    HASH_CA;
	
	pos -> history[pos -> hisPly].move = move;
    pos -> history[pos -> hisPly].fiftyMove = pos -> fiftyMove;
    pos -> history[pos -> hisPly].enPass = pos -> enPass;
    pos -> history[pos -> hisPly].castlePerm = pos -> castlePerm;

    pos -> castlePerm &= castlePerm[from];
    pos -> castlePerm &= castlePerm[to];
    pos -> enPass = NO_SQ;

	HASH_CA;
	
	int captured = CAPTURED(move);
    pos -> fiftyMove++;
	
	if(captured != EMPTY) {
        ASSERT(pieceValid(captured));
        clearPiece(to, pos);
        pos -> fiftyMove = 0;
    }
	
	pos -> hisPly++;
	pos -> ply++;
	
	ASSERT(pos -> hisPly >= 0 && pos -> hisPly < MAXGAMEMOVES);
	ASSERT(pos -> ply >= 0 && pos -> ply < MAXDEPTH);
	
	if (piecePawn[pos -> pieces[from]]) {
        pos -> fiftyMove = 0;
        
        if (move & MFLAGPS) {
            if (side == WHITE) {
                pos -> enPass = from + 10;
                ASSERT(ranksBrd[pos -> enPass] == RANK_3);
            } else {
                pos -> enPass = from - 10;
                ASSERT(ranksBrd[pos -> enPass] == RANK_6);
            }

            HASH_EP;
        }
    }
	
	movePiece(from, to, pos);
	
	int promPiece = PROMOTED(move);

    if (promPiece != EMPTY) {
        ASSERT(pieceValid(promPiece) && !piecePawn[promPiece]);
        clearPiece(to, pos);
        addPiece(to, pos, promPiece);
    }
	
	if (pieceKing[pos -> pieces[to]]) {
        pos -> kingSq[pos -> side] = to;
    }
	
	pos -> side ^= 1;
    HASH_SIDE;

    ASSERT(checkBoard(pos));
	
		
	if (sqAttacked(pos -> kingSq[side], pos -> side, pos)) {
        takeMove(pos);
        return false;
    }
	
	return true;
}

void takeMove(S_BOARD *pos) {
	ASSERT(checkBoard(pos));
	
	pos -> hisPly--;
    pos -> ply--;
	
	ASSERT(pos -> hisPly >= 0 && pos -> hisPly < MAXGAMEMOVES);
	ASSERT(pos -> ply >= 0 && pos -> ply < MAXDEPTH);
	
    int move = pos -> history[pos -> hisPly].move;
    int from = FROMSQ(move);
    int to = TOSQ(move);	
	
	ASSERT(sqOnBoard(from));
    ASSERT(sqOnBoard(to));
	
	if (pos -> enPass != NO_SQ) HASH_EP;
    HASH_CA;

    pos -> castlePerm = pos -> history[pos -> hisPly].castlePerm;
    pos -> fiftyMove = pos -> history[pos -> hisPly].fiftyMove;
    pos -> enPass = pos -> history[pos -> hisPly].enPass;

    if (pos -> enPass != NO_SQ) HASH_EP;
    HASH_CA;

    pos -> side ^= 1;
    HASH_SIDE;
	
	if (MFLAGEP & move) {
        if(pos -> side == WHITE) {
            addPiece(to - 10, pos, bP);
        } else {
            addPiece(to + 10, pos, wP);
        }
    }
    
    else if (MFLAGCA & move) {
        switch (to) {
            case C1: movePiece(D1, A1, pos); break;
            case C8: movePiece(D8, A8, pos); break;
            case G1: movePiece(F1, H1, pos); break;
            case G8: movePiece(F8, H8, pos); break;
            default: ASSERT(false); break;
        }
    }
	
	movePiece(to, from, pos);
	
	if (pieceKing[pos -> pieces[from]]) {
        pos -> kingSq[pos -> side] = from;
    }
	
	int captured = CAPTURED(move);

    if (captured != EMPTY) {
        ASSERT(pieceValid(captured));
        addPiece(to, pos, captured);
    }
	
	if (PROMOTED(move) != EMPTY)   {
        ASSERT(pieceValid(PROMOTED(move)) && !piecePawn[PROMOTED(move)]);
        clearPiece(from, pos);
        addPiece(from, pos, (pieceColor[PROMOTED(move)] == WHITE ? wP : bP));
    }
	
    ASSERT(checkBoard(pos));
}

void makeNullMove(S_BOARD *pos) {

    ASSERT(checkBoard(pos));
    ASSERT(!sqAttacked(pos -> KingSq[pos -> side],pos -> side ^ 1,pos));

    pos -> ply++;
    pos -> history[pos -> hisPly].posKey = pos -> posKey;

    if (pos -> enPass != NO_SQ) HASH_EP;

    pos -> history[pos -> hisPly].move = NOMOVE;
    pos -> history[pos -> hisPly].fiftyMove = pos -> fiftyMove;
    pos -> history[pos -> hisPly].enPass = pos -> enPass;
    pos -> history[pos -> hisPly].castlePerm = pos -> castlePerm;
    pos -> enPass = NO_SQ;

    pos -> side ^= 1;
    pos -> hisPly++;
    HASH_SIDE;
   
    ASSERT(checkBoard(pos));
	ASSERT(pos -> hisPly >= 0 && pos -> hisPly < MAXGAMEMOVES);
	ASSERT(pos -> ply >= 0 && pos -> ply < MAXDEPTH);

    return;
}

void takeNullMove(S_BOARD *pos) {
    ASSERT(checkBoard(pos));

    pos -> hisPly--;
    pos -> ply--;

    if (pos -> enPass != NO_SQ) HASH_EP;

    pos -> castlePerm = pos -> history[pos -> hisPly].castlePerm;
    pos -> fiftyMove = pos -> history[pos -> hisPly].fiftyMove;
    pos -> enPass = pos -> history[pos -> hisPly].enPass;

    if (pos -> enPass != NO_SQ) HASH_EP;
    pos -> side ^= 1;
    HASH_SIDE;
  
    ASSERT(checkBoard(pos));
	ASSERT(pos -> hisPly >= 0 && pos -> hisPly < MAXGAMEMOVES);
	ASSERT(pos -> ply >= 0 && pos -> ply < MAXDEPTH);
}