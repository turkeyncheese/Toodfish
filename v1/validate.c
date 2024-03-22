#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "defs.h"

int moveListOk(const S_MOVELIST *list,  const S_BOARD *pos) {
	if (list -> count < 0 || list -> count >= MAXPOSITIONMOVES) {
		return false;
	}

	int from = 0;
	int to = 0;
	for (int moveNum = 0; moveNum < list -> count; ++moveNum) {
		to = TOSQ(list -> moves[moveNum].move);
		from = FROMSQ(list -> moves[moveNum].move);
		
		if (!sqOnBoard(to) || !sqOnBoard(from)) {
			return false;
		}
		
		if (!pieceValid(pos -> pieces[from])) {
			printBoard(pos);
			return false;
		}
	}

	return true;
}

int sqIs120(const int sq) {
	return (sq>=0 && sq<120);
}

int pieceValidEmptyOffbrd(const int piece) {
	return (pieceValidEmpty(piece) || piece == OFFBOARD);
}
int sqOnBoard(const int sq) {
	return filesBrd[sq] == OFFBOARD ? 0 : 1;
}

int sideValid(const int side) {
	return (side == WHITE || side == BLACK) ? 1 : 0;
}

int fileRankValid(const int fr) {
	return (fr >= 0 && fr <= 7) ? 1 : 0;
}

int pieceValidEmpty(const int piece) {
	return (piece >= EMPTY && piece <= bK) ? 1 : 0;
}

int pieceValid(const int piece) {
	return (piece >= wP && piece <= bK) ? 1 : 0;
}

void debugAnalysisTest(S_BOARD *pos, S_SEARCHINFO *info) {

	FILE *file;
    file = fopen("lct2.epd","r");
    char lineIn [1024];

	info -> depth = MAXDEPTH;
	info -> timeset = true;
	int time = 1140000;


    if (file == NULL) {
        printf("File Not Found\n");
        return;
    } else {
        while (fgets(lineIn , 1024 , file) != NULL) {
			info -> starttime = getTimeMs();
			info -> stoptime = info -> starttime + time;
			clearHashTable(pos -> hashTable);
            parseFEN(lineIn, pos);
            printf("\n%s\n", lineIn);
			printf("time:%d start:%d stop:%d depth:%d timeset:%d\n",
				time, info -> starttime, info -> stoptime, info -> depth, info -> timeset);
			searchPosition(pos, info);
            memset(&lineIn[0], 0, sizeof(lineIn));
        }
    }
}

void mirrorEvalTest(S_BOARD *pos) {
    FILE *file;
    file = fopen("mirror.epd","r");
    char lineIn [1024];
    int ev1 = 0; int ev2 = 0;
    int positions = 0;
    
	if (file == NULL) {
        printf("File Not Found\n");
        return;
    } else {
        while(fgets(lineIn , 1024 , file) != NULL) {
            parseFEN(lineIn, pos);
            positions++;
            ev1 = evalPosition(pos);
            mirrorBoard(pos);
            ev2 = evalPosition(pos);

            if (ev1 != ev2) {
                printf("\n\n\n");
                parseFEN(lineIn, pos);
                printBoard(pos);
                mirrorBoard(pos);
                printBoard(pos);
                printf("\n\nMirror Fail:\n%s\n",lineIn);
                getchar();
                return;
            }

            if ((positions % 1000) == 0)   {
                printf("position %d\n", positions);
            }

            memset(&lineIn[0], 0, sizeof(lineIn));
        }
    }
}