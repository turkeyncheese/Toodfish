#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "defs.h"

#define INPUTBUFFER 400 * 6

void parseGo(char* line, S_SEARCHINFO *info, S_BOARD *pos) {
	int depth = -1, movestogo = 30, movetime = -1;
	int time = -1, inc = 0;
    char *ptr = NULL;
	info -> timeset = false;

	if ((ptr = strstr(line, "infinite"))) {
		;
	}

	if ((ptr = strstr(line, "binc")) && pos -> side == BLACK) {
		inc = atoi(ptr + 5);
	}

	if ((ptr = strstr(line, "winc")) && pos -> side == WHITE) {
		inc = atoi(ptr + 5);
	}

	if ((ptr = strstr(line, "wtime")) && pos -> side == WHITE) {
		time = atoi(ptr + 6);
	}

	if ((ptr = strstr(line, "btime")) && pos -> side == BLACK) {
		time = atoi(ptr + 6);
	}

	if ((ptr = strstr(line, "movestogo"))) {
		movestogo = atoi(ptr + 10);
	}

	if ((ptr = strstr(line, "movetime"))) {
		movetime = atoi(ptr + 9);
	}

	if ((ptr = strstr(line, "depth"))) {
		depth = atoi(ptr + 6);
	}

	if (movetime != -1) {
		time = movetime;
		movestogo = 1;
		info -> timeset = true;
	}

	info -> starttime = getTimeMs();
	info -> depth = depth;

	if (depth != -1 && time != -1) {
		info -> timeset = true;
		time = (getTimeMs() / movestogo);
		time -= 50;

		if (time < 0) {
			time = 0;
			inc -= 50;

			if (inc < 0) inc = 1;
		}

		info -> stoptime = info -> starttime + time + inc;
	}

	if (depth == -1) {
		info -> depth = MAXDEPTH;
	}

	printf("time:%d start:%d stop:%d depth:%d timeset:%d\n",
		time, info -> starttime, info -> stoptime, info -> depth, info -> timeset);
	searchPosition(pos, info);
}

void parsePosition(char* lineIn, S_BOARD *pos) {
	lineIn += 9;
    char *ptrChar = lineIn;

    if (strncmp(lineIn, "startpos", 8) == 0){
        parseFEN(START_FEN, pos);
    } else {
        ptrChar = strstr(lineIn, "fen");
        
        if (ptrChar == NULL) {
            parseFEN(START_FEN, pos);
        } else {
            ptrChar += 4;
            parseFEN(ptrChar, pos);
        }
    }

	ptrChar = strstr(lineIn, "moves");
	int move;

	if (ptrChar != NULL) {
        ptrChar += 6;

        while (*ptrChar) {
              move = parseMove(ptrChar,pos);
			  if (move == NOMOVE) break;
			  makeMove(pos, move);
              pos -> ply = 0;
              while (*ptrChar && *ptrChar!= ' ') ptrChar++;
              ptrChar++;
        }
    }

	printBoard(pos);
}

void uciLoop(S_BOARD *pos, S_SEARCHINFO *info) {
	info -> GAME_MODE = UCIMODE;

	setbuf(stdin, NULL);
    setbuf(stdout, NULL);

	char line[INPUTBUFFER];
    printf("id name %s\n", NAME);
    printf("id author Logan James\n");
	printf("option name Hash type spin default 64 min 4 max %d\n",MAX_HASH);
	printf("option name Book type check default true\n");
    printf("uciok\n");
	
	int MB = 64;

	while (true) {
		memset(&line[0], 0, sizeof(line));
        fflush(stdout);

        if (!fgets(line, INPUTBUFFER, stdin)) continue;

        if (line[0] == '\n') continue;

        if (!strncmp(line, "isready", 7)) {
            printf("readyok\n");
            continue;
        }
        
        else if (!strncmp(line, "position", 8)) {
            parsePosition(line, pos);
        }
        
        else if (!strncmp(line, "ucinewgame", 10)) {
            parsePosition("position startpos\n", pos);
        }
        
        else if (!strncmp(line, "go", 2)) {
            printf("Seen Go..\n");
            parseGo(line, info, pos);
        }
        
        else if (!strncmp(line, "quit", 4)) {
            info -> quit = true;
            break;
        }
        
        else if (!strncmp(line, "uci", 3)) {
            printf("id name %s\n", NAME);
            printf("id author Logan James\n");
            printf("uciok\n");
        }
        
        else if (!strncmp(line, "debug", 4)) {
            debugAnalysisTest(pos, info);
            break;
        }
        
        else if (!strncmp(line, "setoption name Hash value ", 26)) {			
			sscanf(line,"%*s %*s %*s %*s %d", &MB);

			if (MB < 4) MB = 4;
			if (MB > MAX_HASH) MB = MAX_HASH;

			printf("Set Hash to %d MB\n", MB);
			initHashTable(pos -> hashTable, MB);
		}
        
        else if (!strncmp(line, "setoption name Book value ", 26)) {			
			char *ptrtrue = NULL;
			ptrtrue = strstr(line, "true");
			
            if (ptrtrue != NULL) {
				engineOptions -> useBook = true;
			} else {
				engineOptions -> useBook = false;
			}
		}
        
		if (info -> quit) break;
    }
}