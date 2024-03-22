#include <stdbool.h>
#include "defs.h"
#include "polykeys.h"

typedef struct {
	U64 key;
	unsigned short move;
	unsigned short weight;
	unsigned int learn;
	
} S_POLY_BOOK_ENTRY;

long numEntries = 0;

S_POLY_BOOK_ENTRY *entries;

const int polyKindOfPiece[13] = {
	-1, 1, 3, 5, 7, 9, 11, 0, 2, 4, 6, 8, 10
};

void initPolyBook() {

	engineOptions -> useBook = false;
	
	FILE *pFile = fopen("performance.bin", "rb");
	
	if (pFile == NULL) {
		printf("Book File Not Read\n");
	} else {
		fseek(pFile, 0, SEEK_END);
		long position = ftell(pFile);
		
		if (position < sizeof(S_POLY_BOOK_ENTRY)) {
			printf("No Entries Found\n");
			return;
		}
		
		numEntries = position / sizeof(S_POLY_BOOK_ENTRY);
		printf("%ld Entries Found In File\n", numEntries);
		
		entries = (S_POLY_BOOK_ENTRY*) malloc(numEntries * sizeof(S_POLY_BOOK_ENTRY));
		rewind(pFile);
		
		size_t returnValue;
		returnValue = fread(entries, sizeof(S_POLY_BOOK_ENTRY), numEntries, pFile);
		printf("fread() %ld Entries Read in from file\n", returnValue);
		
		if (numEntries > 0) {
			engineOptions -> useBook = true;
		}
	}
}

void cleanPolyBook() {
	free(entries);
}

int hasPawnForCapture(const S_BOARD *board) {
	int sqWithPawn = 0;
	int targetPiece = (board -> side == WHITE) ? wP : bP;

	if (board -> enPass != NO_SQ) {
		if (board -> side == WHITE) {
			sqWithPawn = board -> enPass - 10;
		} else {
			sqWithPawn = board -> enPass + 10;
		}
		
		if (board -> pieces[sqWithPawn + 1] == targetPiece) {
			return true;
		}
        
        else if(board -> pieces[sqWithPawn - 1] == targetPiece) {
			return true;
		} 
	}

	return false;
}

U64 polyKeyFromBoard(const S_BOARD *board) {
	int rank = 0, file = 0;
	U64 finalKey = 0;
	int piece = EMPTY;
	int polyPiece = 0;
	int offset = 0;
	
	for (int sq = 0; sq < BRD_SQ_NUM; ++sq) {
		piece = board -> pieces[sq];

		if (piece != NO_SQ && piece != EMPTY && piece != OFFBOARD) {
			ASSERT(piece >= wP && piece <= bK);
			polyPiece = polyKindOfPiece[piece];
			rank = ranksBrd[sq];
			file = filesBrd[sq];
			finalKey ^= random64Poly[(64 * polyPiece) + (8 * rank) + file];
		}
	}

	offset = 768;
	
	if (board -> castlePerm & WKCA) finalKey ^= random64Poly[offset + 0];
	if (board -> castlePerm & WQCA) finalKey ^= random64Poly[offset + 1];
	if (board -> castlePerm & BKCA) finalKey ^= random64Poly[offset + 2];
	if (board -> castlePerm & BQCA) finalKey ^= random64Poly[offset + 3];

	offset = 772;

	if (hasPawnForCapture(board) == true) {
		file = filesBrd[board -> enPass];
		finalKey ^= random64Poly[offset + file];
	}
	
	if (board -> side == WHITE) {
		finalKey ^= random64Poly[780];
	}
	return finalKey;
}

unsigned short endian_swap_u16(unsigned short x) { 
    x = (x>>8) | 
        (x<<8); 
    return x;
} 

unsigned int endian_swap_u32(unsigned int x) { 
    x = (x>>24) | 
        ((x<<8) & 0x00FF0000) | 
        ((x>>8) & 0x0000FF00) | 
        (x<<24); 
    return x;
} 

U64 endian_swap_u64(U64 x) { 
    x = (x>>56) | 
        ((x<<40) & 0x00FF000000000000) | 
        ((x<<24) & 0x0000FF0000000000) | 
        ((x<<8)  & 0x000000FF00000000) | 
        ((x>>8)  & 0x00000000FF000000) | 
        ((x>>24) & 0x0000000000FF0000) | 
        ((x>>40) & 0x000000000000FF00) | 
        (x<<56); 
    return x;
}

int convertPolyMoveToInternalMove(unsigned short polyMove, S_BOARD *board) {
	int ff = (polyMove >> 6) & 7;
	int fr = (polyMove >> 9) & 7;
	int tf = (polyMove >> 0) & 7;
	int tr = (polyMove >> 3) & 7;
	int pp = (polyMove >> 12) & 7;
	
	char moveString[6];
	if (pp == 0) {
		sprintf(moveString, "%c%c%c%c",
		fileChar[ff],
		rankChar[fr],
		fileChar[tf],
		rankChar[tr]);
	} else {
		char promChar = 'q';

		switch (pp) {
			case 1: promChar = 'n'; break;
			case 2: promChar = 'b'; break;
			case 3: promChar = 'r'; break;
		}

		sprintf(moveString, "%c%c%c%c%c",
		fileChar[ff],
		rankChar[fr],
		fileChar[tf],
		rankChar[tr],
		promChar);
	}
	
	return parseMove(moveString, board);
}

int getBookMove(S_BOARD *board) {
	S_POLY_BOOK_ENTRY *entry;

	unsigned short move;
	const int MAXBOOKMOVES = 32;

	int bookMoves[MAXBOOKMOVES];
	int tempMove = NOMOVE;
	int count = 0;
	
	U64 polyKey = polyKeyFromBoard(board);
	
	for (entry = entries; entry < entries + numEntries; entry++) {
		if (polyKey == endian_swap_u64(entry -> key)) {
			move = endian_swap_u16(entry -> move);
			tempMove = convertPolyMoveToInternalMove(move, board);
			
            if (tempMove != NOMOVE) {
				bookMoves[count++] = tempMove;
				if (count > MAXBOOKMOVES) break;
			}
		}
	}
	
	if (count != 0) {
		int randMove = rand() % count;
		return bookMoves[randMove];
	} else {
		return NOMOVE;
	}
}