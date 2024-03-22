#include <stdbool.h>
#include "defs.h"

char pieceChar[] = ".PNBRQKpnbrqk";
char sideChar[] = "wb-";
char rankChar[] = "12345678";
char fileChar[] = "abcdefgh";

int pieceBig[13] = {false, false, true, true, true, true, true, false, true, true, true, true, true};
int pieceMajor[13] = {false, false, false, false, true, true, true, false, false, false, true, true, true};
int pieceMinor[13] = {false, false, true, true, false, false, false, false, true, true, false, false, false};
int pieceValue[13]= {0, 100, 320, 330, 500, 1000, 50000, 100, 320, 330, 500, 1000, 50000};
int pieceColor[13] = {BOTH, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK};

int piecePawn[13] = {false, true, false, false, false, false, false, true, false, false, false, false, false};	
int pieceKnight[13] = {false, false, true, false, false, false, false, false, true, false, false, false, false};
int pieceKing[13] = {false, false, false, false, false, false, true, false, false, false, false, false, true};
int pieceRookQueen[13] = {false, false, false, false, true, true, false, false, false, false, true, true, false};
int pieceBishopQueen[13] = {false, false, false, true, false, true, false, false, false, true, false, true, false};
int pieceSlides[13] = {false, false, false, true, true, true, false, false, false, true, true, true, false};

int mirror64[64] = {
    56, 57, 58, 59, 60, 61, 62, 63,
    48, 49, 50, 51, 52, 53, 54, 55,
    40, 41, 42, 43, 44, 45, 46, 47,
    32, 33, 34, 35, 36, 37, 38, 39,
    24, 25, 26, 27, 28, 29, 30, 31,
    16, 17, 18, 19, 20, 21, 22, 23,
     8,  9, 10, 11, 12, 13, 14, 15,
     0,  1,  2,  3,  4,  5,  6,  7
};