#include <stdio.h>
#include <stdbool.h>
#include "defs.h"

int getPvLine(const int depth, S_BOARD *pos) {
    ASSERT(depth < MAXDEPTH);

    int move = probePvMove(pos);
	int count = 0;
	
	while (move != NOMOVE && count < depth) {
		ASSERT(count < MAXDEPTH);
	
		if (moveExists(pos, move) ) {
			makeMove(pos, move);
			pos -> pvArray[count++] = move;
		} else {
			break;
		}	

		move = probePvMove(pos);	
	}
	
	while (pos -> ply > 0) {
		takeMove(pos);
	}
	
	return count;
}

void clearHashTable(S_HASHTABLE *table) {
    S_HASHENTRY *tableEntry;

    for (tableEntry = table -> pTable; tableEntry < table -> pTable + table -> numEntries; tableEntry++) {
        tableEntry -> posKey = 0ULL;
        tableEntry -> move = NOMOVE;
        tableEntry -> depth = 0;
        tableEntry -> score = 0;
        tableEntry -> flags = 0;
    }

    table -> newWrite = 0;
}

void initHashTable(S_HASHTABLE *table, const int MB) {
    const int hashSize = 0x100000 * MB;
    table -> numEntries = hashSize / sizeof(S_HASHENTRY);
    table -> numEntries -= 2;

    if (table -> pTable != NULL) {
        free(table -> pTable);
    }

    table -> pTable = (S_HASHENTRY *) malloc(table -> numEntries * sizeof(S_HASHENTRY));

    if (table -> pTable == NULL) {
        printf("Hash Allocation Failed, trying %dMB...\n", MB/2);
		initHashTable(table, MB/2);
    } else {
        clearHashTable(table);
		printf("HashTable init complete with %d entries\n", table -> numEntries);
    }
}

int probeHashEntry(S_BOARD *pos, int *move, int *score, int alpha, int beta, int depth) {
    int index = pos -> posKey % pos -> hashTable -> numEntries;

    ASSERT(index >= 0 && index <= pos -> hashTable -> numEntries - 1);
    ASSERT(depth >= 1&& depth < MAXDEPTH);
    ASSERT(alpha < beta);
    ASSERT(alpha >= -INF && alpha <= INF);
    ASSERT(beta>=-INF && beta <= INF);
    ASSERT(pos -> ply >= 0&& pos -> ply < MAXDEPTH);

    if (pos -> hashTable -> pTable[index].posKey == pos -> posKey ) {
		*move = pos -> hashTable -> pTable[index].move;

		if (pos -> hashTable -> pTable[index].depth >= depth){
			pos -> hashTable -> hit++;
			
			ASSERT(pos -> hashTable -> pTable[index].depth >= 1 && pos -> hashTable -> pTable[index].depth < MAXDEPTH);
            ASSERT(pos -> hashTable -> pTable[index].flags >= HFALPHA && pos -> HashTable -> pTable[index].flags <= HFEXACT);
			
			*score = pos -> hashTable -> pTable[index].score;
			if (*score > ISMATE) *score -= pos -> ply;
            else if (*score < -ISMATE) *score += pos -> ply;
			
			switch (pos -> hashTable -> pTable[index].flags) {
				
                ASSERT(*score >= -INF && *score <= INF);

                case HFALPHA: if (*score<=alpha) {
                    *score = alpha;
                    return true;
                    }
                    break;
                case HFBETA: if(*score>=beta) {
                    *score = beta;
                    return true;
                    }
                    break;
                case HFEXACT:
                    return true;
                    break;
                default: ASSERT(FALSE); break;
            }
		}
	}
	
	return false;
}

void storeHashEntry(S_BOARD *pos, const int move, int score, const int flags, const int depth) {

	int index = pos -> posKey % pos -> hashTable -> numEntries;
	
	ASSERT(index >= 0 && index <= pos -> HashTable -> numEntries - 1);
	ASSERT(depth >= 1 && depth < MAXDEPTH);
    ASSERT(flags >= HFALPHA && flags <= HFEXACT);
    ASSERT(score >= -INF && score <= INF);
    ASSERT(pos -> ply >= 0 && pos -> ply < MAXDEPTH);
	
	if (pos -> hashTable -> pTable[index].posKey == 0) {
		pos -> hashTable -> newWrite++;
	} else {
		pos -> hashTable -> overWrite++;
	}
	
	if (score > ISMATE) score += pos -> ply;
    else if(score < -ISMATE) score -= pos -> ply;
	
	pos -> hashTable -> pTable[index].move = move;
    pos -> hashTable -> pTable[index].posKey = pos -> posKey;
	pos -> hashTable -> pTable[index].flags = flags;
	pos -> hashTable -> pTable[index].score = score;
	pos -> hashTable -> pTable[index].depth = depth;
}

int probePvMove(const S_BOARD *pos) {
	int index = pos -> posKey % pos -> hashTable -> numEntries;
	ASSERT(index >= 0 && index <= pos -> hashTable -> numEntries - 1);
	
	if (pos -> hashTable -> pTable[index].posKey == pos -> posKey ) {
		return pos -> hashTable -> pTable[index].move;
	}
	
	return NOMOVE;
}