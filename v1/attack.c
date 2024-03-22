#include <stdbool.h>
#include "defs.h"

const int knDir[8] = {-8, -19,	-21, -12, 8, 19, 21, 12};
const int rkDir[4] = {-1, -10,	1, 10};
const int biDir[4] = {-9, -11, 11, 9};
const int kiDir[8] = {-1, -10,	1, 10, -9, -11, 11, 9};

int sqAttacked(const int sq, const int side, const S_BOARD *pos) {
    int piece, tempSq, dir;

	ASSERT(sqOnBoard(sq));
	ASSERT(sideValid(side));
	ASSERT(checkBoard(pos));

    if (side == WHITE) {
		if (pos -> pieces[sq - 11] == wP || pos -> pieces[sq - 9] == wP) {
			return true;
		}

	} else {
		if (pos -> pieces[sq + 11] == bP || pos -> pieces[sq + 9] == bP) {
			return true;
		}	
	}

    for (int i = 0; i < 8; ++i) {		
		piece = pos -> pieces[sq + knDir[i]];

		if (piece != OFFBOARD && isKn(piece) && pieceColor[piece] == side) {
			return true;
		}
	}

    for(int i = 0; i < 4; ++i) {		
		dir = rkDir[i];
		tempSq = sq + dir;
		piece = pos -> pieces[tempSq];

		while (piece != OFFBOARD) {
			if (piece != EMPTY) {
				if (isRQ(piece) && pieceColor[piece] == side) {
					return true;
				}

				break;
			}

			tempSq += dir;
			piece = pos -> pieces[tempSq];
		}
	}

    for (int i = 0; i < 4; i++) {		
		dir = biDir[i];
		tempSq = sq + dir;
		piece = pos->pieces[tempSq];

		while (piece != OFFBOARD) {
			if (piece != EMPTY) {
				if (isBQ(piece) && pieceColor[piece] == side) {
					return true;
				}

				break;
            }

			tempSq += dir;
			piece = pos->pieces[tempSq];
		}
	}
	
	for (int i = 0; i < 8; i++) {		
		piece = pos -> pieces[sq + kiDir[i]];

		if (piece != OFFBOARD && isKi(piece) && pieceColor[piece] == side) {
			return true;
		}
	}
	
	return false;
}