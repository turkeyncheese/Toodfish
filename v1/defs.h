#ifndef DEFS_H
#define DEFS_H

#include <stdlib.h>
#include <stdio.h>

#define MAX_HASH 1024

#ifndef DEBUG
#define ASSERT(n)
#else
#define ASSERT(n) \
if(!(n)) { \
printf("%s - Failed",#n); \
printf("On %s ",__DATE__); \
printf("At %s ",__TIME__); \
printf("In File %s ",__FILE__); \
printf("At Line %d\n",__LINE__); \
exit(1);}
#endif

typedef unsigned long long U64;

#define NAME "Toodfish 1.0"
#define BRD_SQ_NUM 120

#define MAXGAMEMOVES 2048
#define MAXPOSITIONMOVES 256
#define MAXDEPTH 64

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#define INF 30000
#define ISMATE (INF - MAXDEPTH)

enum {EMPTY, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK};
enum {FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NONE};
enum {RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NONE};

enum {WHITE, BLACK, BOTH};
enum {UCIMODE, XBOARDMODE, CONSOLEMODE};

enum {
    A1 = 21, B1, C1, D1, E1, F1, G1, H1,
    A2 = 31, B2, C2, D2, E2, F2, G2, H2,
    A3 = 41, B3, C3, D3, E3, F3, G3, H3,
    A4 = 51, B4, C4, D4, E4, F4, G4, H4,
    A5 = 61, B5, C5, D5, E5, F5, G5, H5,
    A6 = 71, B6, C6, D6, E6, F6, G6, H6,
    A7 = 81, B7, C7, D7, E7, F7, G7, H7,
    A8 = 91, B8, C8, D8, E8, F8, G8, H8, NO_SQ, OFFBOARD
};

enum {WKCA = 1, WQCA = 2, BKCA = 4, BQCA = 8};

typedef struct {
    int move;
    int score;
} S_MOVE;

typedef struct {
    S_MOVE moves[MAXPOSITIONMOVES];
    int count;
} S_MOVELIST;

typedef struct {
    int move;
    int castlePerm;
    int enPass;
    int fiftyMove;
    
    U64 posKey;
} S_UNDO;

enum {HFNONE, HFALPHA, HFBETA, HFEXACT};

typedef struct {
    U64 posKey;
    int move;
    int score;
    int depth;
    int flags;
} S_HASHENTRY;

typedef struct {
	S_HASHENTRY *pTable;
	int numEntries;
	int newWrite;
	int overWrite;
	int hit;
	int cut;
} S_HASHTABLE;

typedef struct {
    int pieces[BRD_SQ_NUM];
    U64 pawns[3];

    int kingSq[2];

    int side;
    int enPass;
    int fiftyMove;

    int ply;
    int hisPly;

    int castlePerm;

    U64 posKey;

    int pieceNum[13];
    int bigPieces[2];
    int majorPieces[2];
    int minorPieces[2];
    int material[2];

    S_UNDO history[MAXGAMEMOVES];

    int pieceList[13][10];

    S_HASHTABLE hashTable[1];
    int pvArray[MAXDEPTH];

    int searchHistory[13][BRD_SQ_NUM];
    int searchKillers[2][MAXDEPTH];
} S_BOARD;

typedef struct {
	int starttime;
	int stoptime;
	int depth;
	int timeset;
	int movestogo;
    int infinite;

	long nodes;

	int quit;
	int stopped;

	float fh;
	float fhf;
	int nullCut;

	int GAME_MODE;
	int POST_THINKING;
} S_SEARCHINFO;

typedef struct {
	int useBook;
} S_OPTIONS;

/* Game Move*/
#define FROMSQ(m) (m & 0x7F)
#define TOSQ(m) ((m >> 7) & 0x7F)
#define CAPTURED(m) ((m >> 14) & 0xF)
#define PROMOTED(m) ((m >> 20) & 0xF)

#define MFLAGEP 0x40000
#define MFLAGPS 0x80000
#define MFLAGCA 0x1000000

#define MFLAGCAP 0x7C000
#define MFLAGPROM 0xF00000

#define NOMOVE 0

/* Macros */
#define FR2SQ(f, r) ((21 + f) + (r * 10))
#define SQ64(sq120) sq120ToSq64[sq120]
#define SQ120(sq64) sq64ToSq120[sq64]

#define POP(b) popBit(b)
#define COUNT(b) countBits(b)

#define CLEARBIT(bb, sq) (bb &= clearMask[sq])
#define SETBIT(bb, sq) (bb |= setMask[sq])

#define isBQ(p) pieceBishopQueen[p]
#define isRQ(p) pieceRookQueen[p]
#define isKn(p) pieceKnight[p]
#define isKi(p) pieceKing[p]

#define MIRROR64(sq) (mirror64[sq])

/* Globals */
extern int sq120ToSq64[BRD_SQ_NUM];
extern int sq64ToSq120[64];

extern U64 setMask[64];
extern U64 clearMask[64];

extern U64 pieceKeys[13][120];
extern U64 sideKey;
extern U64 castleKeys[16];

extern char pieceChar[];
extern char sideChar[];
extern char rankChar[];
extern char fileChar[];

extern int pieceBig[13];
extern int pieceMajor[13];
extern int pieceMinor[13];
extern int pieceValue[13];
extern int pieceColor[13];

extern int filesBrd[BRD_SQ_NUM];
extern int ranksBrd[BRD_SQ_NUM];

extern int piecePawn[13];
extern int pieceKnight[13];
extern int pieceKing[13];
extern int pieceRookQueen[13];
extern int pieceBishopQueen[13];
extern int pieceSlides[13];

extern int mirror64[64];

extern U64 fileBBMask[8];
extern U64 rankBBMask[8];

extern U64 blackPassedMask[64];
extern U64 whitePassedMask[64];
extern U64 isolatedMask[64];

extern S_OPTIONS engineOptions[1];

/* Functions */

// init.c
extern void initAll();

// bitboards.c
extern void printBitBoard(U64 bb);
extern int popBit(U64 *bb);
extern int countBits(U64 b);

// hashkeys.c
extern U64 generatePosKey(const S_BOARD *pos);

// board.c
extern void resetBoard(S_BOARD *pos);
extern int parseFEN(char *fen, S_BOARD *pos);
extern void printBoard(const S_BOARD *pos);
extern void updateListsMaterial(S_BOARD *pos);
extern int checkBoard(const S_BOARD *pos);
extern void mirrorBoard(S_BOARD *pos);

// attack.c
extern int sqAttacked(const int sq, const int side, const S_BOARD *pos);

// io.c
extern char *printSq(const int sq);
extern char *printMove(const int move);
extern void printMoveList(const S_MOVELIST *list);
extern int parseMove(char *ptrchar, S_BOARD *pos);

// validate.c
extern int sqOnBoard(const int sq);
extern int sideValid(const int side);
extern int fileRankValid(const int fr);
extern int pieceValidEmpty(const int piece);
extern int pieceValid(const int piece);
extern void mirrorEvalTest(S_BOARD *pos);
extern int sqIs120(const int sq);
extern int pieceValidEmptyOffbrd(const int piece);
extern int moveListOk(const S_MOVELIST *list,  const S_BOARD *pos);
extern void debugAnalysisTest(S_BOARD *pos, S_SEARCHINFO *info);

// movegen.c
extern void generateAllMoves(const S_BOARD *pos, S_MOVELIST *list);
extern void generateAllCaps(const S_BOARD *pos, S_MOVELIST *list);
extern int moveExists(S_BOARD *pos, const int move);
extern void initMvvLva();

// makemove.c
extern int makeMove(S_BOARD *pos, int move);
extern void takeMove(S_BOARD *pos);
extern void makeNullMove(S_BOARD *pos);
extern void takeNullMove(S_BOARD *pos);

// perft.c
extern void perftTest(int depth, S_BOARD *pos);

// search.c
extern void searchPosition(S_BOARD *pos, S_SEARCHINFO *info);

// misc.c
extern int getTimeMs();
extern void readInput(S_SEARCHINFO *info);

// pvtable.c
extern void initHashTable(S_HASHTABLE *table, const int MB);
extern void storeHashEntry(S_BOARD *pos, const int move, int score, const int flags, const int depth);
extern int probeHashEntry(S_BOARD *pos, int *move, int *score, int alpha, int beta, int depth);
extern int probePvMove(const S_BOARD *pos);
extern int getPvLine(const int depth, S_BOARD *pos);
extern void clearHashTable(S_HASHTABLE *table);

// evaluate.c
extern int evalPosition(const S_BOARD *pos);
extern void mirrorEvalTest(S_BOARD *pos);

// uci.c
extern void uciLoop(S_BOARD *pos, S_SEARCHINFO *info);

// xboard.c
extern void xBoardLoop(S_BOARD *pos, S_SEARCHINFO *info);
extern void consoleLoop(S_BOARD *pos, S_SEARCHINFO *info);

// polybook.c
extern int getBookMove(S_BOARD *board);
extern void cleanPolyBook();
extern void initPolyBook();

#endif