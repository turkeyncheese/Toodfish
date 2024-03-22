#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "defs.h"

#define WAC1 "2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3Q1/PPB4P/R4RK1 w - -"
#define PERFT "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"

int main(int argc, char *argv[]) {
    initAll();

    S_BOARD pos[1];
    S_SEARCHINFO info[1];

    info -> quit = false;
	pos -> hashTable -> pTable = NULL;

    initHashTable(pos -> hashTable, 64);
	setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    
    int argNum = 0;
    
    for (argNum = 0; argNum < argc; argNum++) {
    	if (strncmp(argv[argNum], "NoBook", 6) == 0) {
    		engineOptions -> useBook = false;
    		printf("Book Off\n");
    	}
    }

	printf("Welcome to Toodfish! Type 'toodfish' for console mode...\n");

	char line[256];

	while (true) {
		memset(&line[0], 0, sizeof(line));

		fflush(stdout);
		if (!fgets(line, 256, stdin))
			continue;

		if (line[0] == '\n')
			continue;

		if (!strncmp(line, "uci", 3)) {
			uciLoop(pos, info);
			if (info -> quit == true) break;
			continue;
		}
        
        else if (!strncmp(line, "xboard", 6))	{
			xBoardLoop(pos, info);
			if (info -> quit == true) break;
			continue;
		}
        
        else if (!strncmp(line, "toodfish", 4))	{
			consoleLoop(pos, info);
			if (info -> quit == true) break;
			continue;
		}
        
        else if (!strncmp(line, "quit", 4))	{
			break;
		}
	}

	free(pos -> hashTable -> pTable);
	cleanPolyBook();
    
	return 0;
}