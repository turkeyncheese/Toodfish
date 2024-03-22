#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "defs.h"

int threeFoldRep(const S_BOARD *pos) {
	ASSERT(checkBoard(pos));

	int r = 0;
	for (int i = 0; i < pos -> hisPly; ++i)	{
	    if (pos -> history[i].posKey == pos -> posKey) {
		    r++;
		}
	}
	return r;
}

int drawMaterial(const S_BOARD *pos) {
	ASSERT(checkBoard(pos));

    if (pos -> pieceNum[wP] || pos -> pieceNum[bP]) return false;
    if (pos -> pieceNum[wQ] || pos -> pieceNum[bQ] || pos -> pieceNum[wR] || pos -> pieceNum[bR]) return false;
    if (pos -> pieceNum[wB] > 1 || pos -> pieceNum[bB] > 1) {return false;}
    if (pos -> pieceNum[wN] > 1 || pos -> pieceNum[bN] > 1) {return false;}
    if (pos -> pieceNum[wN] && pos -> pieceNum[wB]) {return false;}
    if (pos -> pieceNum[bN] && pos -> pieceNum[bB]) {return false;}

    return true;
}

int checkResult(S_BOARD *pos) {
	ASSERT(checkBoard(pos));

    if (pos -> fiftyMove > 100) {
     	printf("1/2-1/2 {fifty move rule (claimed by Toodfish)}\n"); return true;
    }

    if (threeFoldRep(pos) >= 2) {
     	printf("1/2-1/2 {3-fold repetition (claimed by Toodfish)}\n"); return true;
    }

	if (drawMaterial(pos) == true) {
     	printf("1/2-1/2 {insufficient material (claimed by Toodfish)}\n"); return true;
    }

	S_MOVELIST list[1];
    generateAllMoves(pos,list);

	int found = 0;
	for (int moveNum = 0; moveNum < list -> count; ++moveNum) {

        if (!makeMove(pos, list -> moves[moveNum].move))  {
            continue;
        }

        found++;
		takeMove(pos);
		break;
    }

	if (found != 0) return false;

	int inCheck = sqAttacked(pos -> kingSq[pos -> side], pos -> side ^ 1, pos);

	if (inCheck == true)	{
	    if (pos -> side == WHITE) {
	      	printf("0-1 {black mates (claimed by Toodfish)}\n"); return true;
        } else {
	      	printf("0-1 {white mates (claimed by Toodfish)}\n"); return true;
        }
    } else {
      	printf("\n1/2-1/2 {stalemate (claimed by Toodfish)}\n"); return true;
    }

	return false;
}

void printOptions() {
	printf("feature ping=1 setboard=1 colors=0 usermove=1 memory=1\n");
	printf("feature done=1\n");
}

void xBoardLoop(S_BOARD *pos, S_SEARCHINFO *info) {
	info -> GAME_MODE = XBOARDMODE;
	info -> POST_THINKING = true;

	setbuf(stdin, NULL);
    setbuf(stdout, NULL);
	printOptions();

	int depth = -1, movestogo[2] = {30,30 }, movetime = -1;
	int time = -1, inc = 0;
	int engineSide = BOTH;
	int timeLeft;
	int sec;
	int mps;
	int move = NOMOVE;
	char inBuf[80], command[80];
	int MB;

	engineSide = BLACK;
	parseFEN(START_FEN, pos);
	depth = -1;
	time = -1;

	while (true) {
		fflush(stdout);

		if (pos -> side == engineSide && checkResult(pos) == false) {
			info -> starttime = getTimeMs();
			info -> depth = depth;

			if (time != -1) {
				info -> timeset = true;
				time /= movestogo[pos -> side];
				time -= 50;
				info -> stoptime = info -> starttime + time + inc;
			}

			if (depth == -1 || depth > MAXDEPTH) {
				info -> depth = MAXDEPTH;
			}

			printf("time:%d start:%d stop:%d depth:%d timeset:%d movestogo:%d mps:%d\n",
				time, info -> starttime, info -> stoptime, info -> depth, info -> timeset, movestogo[pos -> side], mps);
				searchPosition(pos, info);

			if (mps != 0) {
				movestogo[pos -> side^1]--;
				
				if (movestogo[pos -> side ^ 1] < 1) {
					movestogo[pos -> side ^ 1] = mps;
				}
			}

		}

		fflush(stdout);

		memset(&inBuf[0], 0, sizeof(inBuf));
		fflush(stdout);
		if (!fgets(inBuf, 80, stdin))
		continue;

		sscanf(inBuf, "%s", command);

		printf("command seen:%s\n", inBuf);

		if (!strcmp(command, "quit")) {
			info -> quit = true;
			break;
		}

		if (!strcmp(command, "force")) {
			engineSide = BOTH;
			continue;
		}

		if (!strcmp(command, "protover")){
			printOptions();
		    continue;
		}

		if (!strcmp(command, "sd")) {
			sscanf(inBuf, "sd %d", &depth);
		    printf("DEBUG depth:%d\n", depth);
			continue;
		}

		if (!strcmp(command, "st")) {
			sscanf(inBuf, "st %d", &movetime);
		    printf("DEBUG movetime:%d\n", movetime);
			continue;
		}

		if (!strcmp(command, "time")) {
			sscanf(inBuf, "time %d", &time);
			time *= 10;
		    printf("DEBUG time:%d\n", time);
			continue;
		}
		
		if (!strcmp(command, "memory")) {			
			sscanf(inBuf, "memory %d", &MB);	

		    if (MB < 4) MB = 4;
			if (MB > MAX_HASH) MB = MAX_HASH;

			printf("Set Hash to %d MB\n", MB);
			initHashTable(pos -> hashTable, MB);
			continue;
		}

		if (!strcmp(command, "level")) {
			sec = 0;
			movetime = -1;

			if (sscanf(inBuf, "level %d %d %d", &mps, &timeLeft, &inc) != 3) {
			  sscanf(inBuf, "level %d %d:%d %d", &mps, &timeLeft, &sec, &inc);
		      printf("DEBUG level with :\n");
			} else {
		      printf("DEBUG level without :\n");
			}

			timeLeft *= 60000;
			timeLeft += sec * 1000;
			movestogo[0] = movestogo[1] = 30;

			if (mps != 0) {
				movestogo[0] = movestogo[1] = mps;
			}

			time = -1;
		    printf("DEBUG level timeLeft:%d movesToGo:%d inc:%d mps%d\n", timeLeft, movestogo[0], inc, mps);
			continue;
		}

		if (!strcmp(command, "ping")) {
			printf("pong%s\n", inBuf + 4);
			continue;
		}

		if (!strcmp(command, "new")) {
			clearHashTable(pos -> hashTable);
			engineSide = BLACK;
			parseFEN(START_FEN, pos);
			depth = -1;
			time = -1;
			continue;
		}

		if (!strcmp(command, "setboard")){
			engineSide = BOTH;
			parseFEN(inBuf + 9, pos);
			continue;
		}

		if (!strcmp(command, "go")) {
			engineSide = pos -> side;
			continue;
		}

		if (!strcmp(command, "usermove")){
			movestogo[pos -> side]--;
			move = parseMove(inBuf + 9, pos);
			if(move == NOMOVE) continue;
			makeMove(pos, move);
            pos -> ply=0;
		}
    }
}


void consoleLoop(S_BOARD *pos, S_SEARCHINFO *info) {
	printf("Welcome to Toodfish In Console Mode!\n");
	printf("Type help for commands\n\n");

	info -> GAME_MODE = CONSOLEMODE;
	info -> POST_THINKING = true;

	setbuf(stdin, NULL);
    setbuf(stdout, NULL);

	int depth = MAXDEPTH, movetime = 3000;
	int engineSide = BOTH;
	int move = NOMOVE;
	char inBuf[80], command[80];

	engineSide = BLACK;
	parseFEN(START_FEN, pos);

	while (true) {
		fflush(stdout);

		if (pos -> side == engineSide && checkResult(pos) == false) {
			info -> starttime = getTimeMs();
			info -> depth = depth;

			if (movetime != 0) {
				info -> timeset = true;
				info -> stoptime = info -> starttime + movetime;
			}

			searchPosition(pos, info);
		}

		printf("\nToodfish > ");

		fflush(stdout);

		memset(&inBuf[0], 0, sizeof(inBuf));
		fflush(stdout);
		if (!fgets(inBuf, 80, stdin))
		continue;

		sscanf(inBuf, "%s", command);

		if (!strcmp(command, "help")) {
			printf("Commands:\n");
			printf("quit - quit game\n");
			printf("force - computer will not think\n");
			printf("print - show board\n");
			printf("post - show thinking\n");
			printf("nopost - do not show thinking\n");
			printf("new - start new game\n");
			printf("go - set computer thinking\n");
			printf("depth x - set depth to x\n");
			printf("time x - set thinking time to x seconds (depth still applies if set)\n");
			printf("view - show current depth and movetime settings\n");
			printf("setboard x - set position to fen x\n");
			printf("** note ** - to reset time and depth, set to 0\n");
			printf("enter moves using b7b8q notation\n\n\n");
			continue;
		}

		if (!strcmp(command, "mirror")) {
			engineSide = BOTH;
			mirrorEvalTest(pos);
			continue;
		}

		if (!strcmp(command, "eval")) {
			printBoard(pos);
			printf("Eval:%d", evalPosition(pos));
			mirrorBoard(pos);
			printBoard(pos);
			printf("Eval:%d", evalPosition(pos));
			continue;
		}

		if (!strcmp(command, "setboard")){
			engineSide = BOTH;
			parseFEN(inBuf + 9, pos);
			continue;
		}

		if (!strcmp(command, "quit")) {
			info -> quit = true;
			break;
		}

		if (!strcmp(command, "post")) {
			info -> POST_THINKING = true;
			continue;
		}

		if (!strcmp(command, "print")) {
			printBoard(pos);
			continue;
		}

		if (!strcmp(command, "nopost")) {
			info -> POST_THINKING = false;
			continue;
		}

		if (!strcmp(command, "force")) {
			engineSide = BOTH;
			continue;
		}

		if (!strcmp(command, "view")) {
			if (depth == MAXDEPTH) printf("depth not set ");
			else printf("depth %d",depth);

			if (movetime != 0) printf(" movetime %ds\n", movetime / 1000);
			else printf(" movetime not set\n");

			continue;
		}

		if (!strcmp(command, "depth")) {
			sscanf(inBuf, "depth %d", &depth);
		    if (depth == 0) depth = MAXDEPTH;
			continue;
		}

		if (!strcmp(command, "time")) {
			sscanf(inBuf, "time %d", &movetime);
			movetime *= 1000;
			continue;
		}

		if (!strcmp(command, "new")) {
			clearHashTable(pos -> hashTable);
			engineSide = BLACK;
			parseFEN(START_FEN, pos);
			continue;
		}

		if (!strcmp(command, "go")) {
			engineSide = pos -> side;
			continue;
		}

		move = parseMove(inBuf, pos);
		if (move == NOMOVE) {
			printf("Command unknown:%s\n", inBuf);
			continue;
		}

		makeMove(pos, move);
		pos -> ply = 0;
    }
}