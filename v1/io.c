#include <stdio.h>
#include "defs.h"

char *printSq(const int sq) {
    static char sqStr[3];

    int file = filesBrd[sq];
    int rank = ranksBrd[sq];

    sprintf(sqStr, "%c%c", ('a' + file), '1' + rank);

    return sqStr;
}

char *printMove(const int move) {
    static char moveStr[6];

	int ff = filesBrd[FROMSQ(move)];
	int rf = ranksBrd[FROMSQ(move)];
	int ft = filesBrd[TOSQ(move)];
	int rt = ranksBrd[TOSQ(move)];

	int promoted = PROMOTED(move);

	if (promoted) {
		char pieceChar = 'q';

		if (isKn(promoted)) {
			pieceChar = 'n';
		}
        
        else if (isRQ(promoted) && !isBQ(promoted))  {
			pieceChar = 'r';
		}
        
        else if(!isRQ(promoted) && isBQ(promoted))  {
			pieceChar = 'b';
		}

		sprintf(moveStr, "%c%c%c%c%c", ('a' + ff), ('1' + rf), ('a' + ft), ('1' + rt), pieceChar);
	} else {
		sprintf(moveStr, "%c%c%c%c", ('a' + ff), ('1' + rf), ('a' + ft), ('1' + rt));
	}

	return moveStr;
}

int parseMove(char *ptrChar, S_BOARD *pos) {
	ASSERT(checkBoard(pos));

	if (ptrChar[1] > '8' || ptrChar[1] < '1') return NOMOVE;
    if (ptrChar[3] > '8' || ptrChar[3] < '1') return NOMOVE;
    if (ptrChar[0] > 'h' || ptrChar[0] < 'a') return NOMOVE;
    if (ptrChar[2] > 'h' || ptrChar[2] < 'a') return NOMOVE;

    int from = FR2SQ(ptrChar[0] - 'a', ptrChar[1] - '1');
    int to = FR2SQ(ptrChar[2] - 'a', ptrChar[3] - '1');

	ASSERT(sqOnBoard(from) && sqOnBoard(to));

	S_MOVELIST list[1];
    generateAllMoves(pos, list);

	int move = 0;
	int promPiece = EMPTY;

	for (int moveNum = 0; moveNum < list->count; ++moveNum) {
		move = list->moves[moveNum].move;

		if (FROMSQ(move) == from && TOSQ(move) == to) {
			promPiece = PROMOTED(move);

			if (promPiece != EMPTY) {
				if (isRQ(promPiece) && !isBQ(promPiece) && ptrChar[4] == 'r') {
					return move;
				}
				
				else if (!isRQ(promPiece) && isBQ(promPiece) && ptrChar[4] == 'b') {
					return move;
				}
				
				else if (isRQ(promPiece) && isBQ(promPiece) && ptrChar[4] == 'q') {
					return move;
				}
				
				else if (isKn(promPiece)&& ptrChar[4] == 'n') {
					return move;
				}
				continue;
			}

			return move;
		}
    }

    return NOMOVE;
}

void printMoveList(const S_MOVELIST *list) {
	int score = 0;
	int move = 0;

	printf("MoveList:\n");

	for (int i = 0; i < list->count; ++i) {

		move = list -> moves[i].move;
		score = list -> moves[i].score;

		printf("Move:%d > %s (score:%d)\n", i + 1, printMove(move), score);
	}

	printf("MoveList Total %d Moves:\n\n", list -> count);
}