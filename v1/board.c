#include <stdio.h>
#include <stdbool.h>
#include "defs.h"

int pieceListOk(const S_BOARD *pos) {
	int sq;

	for (int i = wP; i <= bK; i++) {
		if (pos -> pieceNum[i] < 0 || pos -> pieceNum[i] >= 10) return false;
	}

	if (pos -> pieceNum[wK] != 1 || pos -> pieceNum[bK] != 1) return false;

	for (int i = wP; i <= bK; i++) {
		for (int j = 0; j < pos -> pieceNum[i]; j++) {
			sq = pos -> pieceList[i][j];

			if (!sqOnBoard(sq)) return false;
		}
	}

    return true;
}

int checkBoard(const S_BOARD *pos) {
	int tempPieceNum[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int tempPieceBig[2] = {0, 0};
	int tempPieceMajor[2] = {0, 0};
	int tempPieceMinor[2] = {0, 0};
	int tempMaterial[2] = {0, 0};

	int sq64, tempPiece, sq120, color, pcount;

	U64 tempPawns[3] = {0ULL, 0ULL, 0ULL};

	tempPawns[WHITE] = pos -> pawns[WHITE];
	tempPawns[BLACK] = pos -> pawns[BLACK];
	tempPawns[BOTH] = pos -> pawns[BOTH];

	for (int i = wP; i <= bK; i++) {
		for (int j = 0; j < pos -> pieceNum[i]; j++) {
			sq120 = pos -> pieceList[i][j];
			ASSERT(pos -> pieces[sq120] == i);
		}
	}

	for (int i = 0; i < 64; i++) {
		sq120 = SQ120(i);
		tempPiece = pos -> pieces[sq120];
		tempPieceNum[tempPiece]++;
		color = pieceColor[tempPiece];

		if (pieceBig[tempPiece] == true) tempPieceBig[color]++;
		if (pieceMinor[tempPiece] == true) tempPieceMinor[color]++;
		if (pieceMajor[tempPiece] == true) tempPieceMajor[color]++;

		tempMaterial[color] += pieceValue[tempPiece];
	}

	for (int i = wP; i <= bK; i++) {
		ASSERT(tempPieceNum[tempPiece] == pos -> pieceNum[tempPiece]);
	}

	pcount = COUNT(tempPawns[WHITE]);
	ASSERT(pcount == pos -> pceNum[wP]);

	pcount = COUNT(tempPawns[BLACK]);
	ASSERT(pcount == pos -> pceNum[bP]);

	pcount = COUNT(tempPawns[BOTH]);
	ASSERT(pcount == (pos -> pceNum[bP] + pos -> pceNum[wP]));

	while (tempPawns[WHITE]) {
		sq64 = POP(&tempPawns[WHITE]);
		ASSERT(pos -> pieces[SQ120(sq64)] == wP);
	}

	while (tempPawns[BLACK]) {
		sq64 = POP(&tempPawns[BLACK]);
		ASSERT(pos -> pieces[SQ120(sq64)] == bP);
	}

	while (tempPawns[BOTH]) {
		sq64 = POP(&tempPawns[BOTH]);
		ASSERT((pos -> pieces[SQ120(sq64)] == bP) || (pos -> pieces[SQ120(sq64)] == wP));
	}

	ASSERT(tempMaterial[WHITE] == pos -> material[WHITE] && tempMaterial[BLACK] == pos -> material[BLACK]);
	ASSERT(tempPieceMinor[WHITE] == pos -> pieceMinor[WHITE] && tempPieceMinor[BLACK] == pos -> pieceMinor[BLACK]);
	ASSERT(tempPieceMajor[WHITE] == pos -> pieceMajor[WHITE] && tempPieceMajor[BLACK] == pos -> pieceMajor[BLACK]);
	ASSERT(tempPieceBig[WHITE] == pos -> pieceBig[WHITE] && tempPieceBig[BLACK] == pos -> pieceBig[BLACK]);

	ASSERT(pos -> side == WHITE || pos -> side == BLACK);
	ASSERT(generatePosKey(pos) == pos -> posKey);

	ASSERT(pos -> enPass == NO_SQ || ( ranksBrd[pos -> enPass] == RANK_6 && pos -> side == WHITE)
		 || ( ranksBrd[pos -> enPass] == RANK_3 && pos -> side == BLACK));

	ASSERT(pos -> pieces[pos -> kingSq[WHITE]] == wK);
	ASSERT(pos -> pieces[pos -> kingSq[BLACK]] == bK);

	ASSERT(pos -> castlePerm >= 0 && pos -> castlePerm <= 15);

	ASSERT(pieceListOk(pos));

	return true;
}

void updateListsMaterial(S_BOARD *pos) {

	int piece, sq, color;

	for(int i = 0; i < BRD_SQ_NUM; ++i) {
		sq = i;
		piece = pos -> pieces[i];

		ASSERT(pieceValidEmptyOffbrd(piece));

		if (piece != OFFBOARD && piece != EMPTY) {
			color = pieceColor[piece];
			ASSERT(sideValid(color));

		    if (pieceBig[piece] == true) pos -> bigPieces[color]++;
		    if (pieceMinor[piece] == true) pos -> minorPieces[color]++;
		    if (pieceMajor[piece] == true) pos -> majorPieces[color]++;

			pos -> material[color] += pieceValue[piece];

			ASSERT(pos -> pieceNum[piece] < 10 && pos -> pieceNum[piece] >= 0);

			pos -> pieceList[piece][pos -> pieceNum[piece]] = sq;
			pos -> pieceNum[piece]++;

			if (piece == wK) pos -> kingSq[WHITE] = sq;
			if (piece == bK) pos -> kingSq[BLACK] = sq;

			if (piece == wP) {
				SETBIT(pos -> pawns[WHITE], SQ64(sq));
				SETBIT(pos -> pawns[BOTH], SQ64(sq));
			}
            
            else if (piece == bP) {
				SETBIT(pos -> pawns[BLACK], SQ64(sq));
				SETBIT(pos -> pawns[BOTH], SQ64(sq));
			}
		}
	}
}

int parseFEN(char *fen, S_BOARD *pos) {
    ASSERT(fen != NULL);
	ASSERT(pos != NULL);

	int rank = RANK_8;
    int file = FILE_A;
    int piece = 0;
    int count = 0;
	int sq64 = 0;
	int sq120 = 0;

	resetBoard(pos);

	while ((rank >= RANK_1) && *fen) {
	    count = 1;

		switch (*fen) {
            case 'p': piece = bP; break;
            case 'r': piece = bR; break;
            case 'n': piece = bN; break;
            case 'b': piece = bB; break;
            case 'k': piece = bK; break;
            case 'q': piece = bQ; break;
            case 'P': piece = wP; break;
            case 'R': piece = wR; break;
            case 'N': piece = wN; break;
            case 'B': piece = wB; break;
            case 'K': piece = wK; break;
            case 'Q': piece = wQ; break;

            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                piece = EMPTY;
                count = *fen - '0';
                break;

            case '/':
            case ' ':
                rank--;
                file = FILE_A;
                fen++;
                continue;

            default:
                printf("FEN error \n");
                return -1;
        }

		for (int i = 0; i < count; i++) {
            sq64 = rank * 8 + file;
			sq120 = SQ120(sq64);

            if (piece != EMPTY) {
                pos -> pieces[sq120] = piece;
            }

			file++;
        }

		fen++;
	}

	ASSERT(*fen == 'w' || *fen == 'b');

	pos -> side = (*fen == 'w') ? WHITE : BLACK;
	fen += 2;

	for (int i = 0; i < 4; i++) {
        if (*fen == ' ') {
            break;
        }

		switch(*fen) {
			case 'K': pos -> castlePerm |= WKCA; break;
			case 'Q': pos -> castlePerm |= WQCA; break;
			case 'k': pos -> castlePerm |= BKCA; break;
			case 'q': pos -> castlePerm |= BQCA; break;
			default: break;
        }

		fen++;
	}

	fen++;

	ASSERT(pos -> castlePerm >= 0 && pos -> castlePerm <= 15);

	if (*fen != '-') {
		file = fen[0] - 'a';
		rank = fen[1] - '1';

		ASSERT(file >= FILE_A && file <= FILE_H);
		ASSERT(rank >= RANK_1 && rank <= RANK_8);

		pos -> enPass = FR2SQ(file,rank);
    }

	pos -> posKey = generatePosKey(pos);

    updateListsMaterial(pos);

	return 0;
}

void printBoard(const S_BOARD *pos) {
    int sq, piece;

    for (int rank = RANK_8; rank >= RANK_1; rank--) {
        for (int file = FILE_A; file <= FILE_H; file++) {
            sq = FR2SQ(file, rank);
            piece = pos -> pieces[sq];

            if (file == FILE_A) {
                printf("  %d  ", rank + 1);
            }

            printf("%c ", pieceChar[piece]);
        }

        printf("\n");
    }

    printf("\n     A B C D E F G H\n\n");

    printf("     Side:        %c\n", sideChar[pos -> side]);
    printf("     En Passant:  %d\n", pos -> enPass);
    printf("     Castling:    %c%c%c%c\n", pos -> castlePerm & WKCA ? 'K' : '-',
                                           pos -> castlePerm & WQCA ? 'Q' : '-',
                                           pos -> castlePerm & BKCA ? 'k' : '-',
                                           pos -> castlePerm & BQCA ? 'q' : '-');
    printf("     PosKey:      %llX\n\n", pos -> posKey);
}

void resetBoard(S_BOARD *pos) {
    for (int i = 0; i < BRD_SQ_NUM; i++) {
        pos -> pieces[i] = OFFBOARD;
    }

    for (int i = 0; i < 64; i++) {
        pos -> pieces[SQ120(i)] = EMPTY;
    }

    for (int i = 0; i < 2; i++) {
		pos -> bigPieces[i] = 0;
		pos -> majorPieces[i] = 0;
		pos -> minorPieces[i] = 0;
        pos -> material[i] = 0;
	}

	for (int i = 0; i < 3; ++i) {
		pos -> pawns[i] = 0ULL;
	}

	for (int i = 0; i < 13; ++i) {
		pos -> pieceNum[i] = 0;
	}

	pos -> kingSq[WHITE] = pos -> kingSq[BLACK] = NO_SQ;

	pos -> side = BOTH;
	pos -> enPass = NO_SQ;
	pos -> fiftyMove = 0;

	pos -> ply = 0;
	pos -> hisPly = 0;

	pos -> castlePerm = 0;

	pos -> posKey = 0ULL;
}

void mirrorBoard(S_BOARD *pos) {
    int tempPiecesArray[64];
    int tempSide = pos->side^1;
	int swapPiece[13] = { EMPTY, bP, bN, bB, bR, bQ, bK, wP, wN, wB, wR, wQ, wK };
    int tempCastlePerm = 0;
    int tempEnPass = NO_SQ;

	int tp;

    if (pos -> castlePerm & WKCA) tempCastlePerm |= BKCA;
    if (pos -> castlePerm & WQCA) tempCastlePerm |= BQCA;

    if (pos -> castlePerm & BKCA) tempCastlePerm |= WKCA;
    if (pos -> castlePerm & BQCA) tempCastlePerm |= WQCA;

	if (pos -> enPass != NO_SQ)  {
        tempEnPass = SQ120(mirror64[SQ64(pos -> enPass)]);
    }

    for (int sq = 0; sq < 64; sq++) {
        tempPiecesArray[sq] = pos -> pieces[SQ120(mirror64[sq])];
    }

    resetBoard(pos);

	for (int sq = 0; sq < 64; sq++) {
        tp = swapPiece[tempPiecesArray[sq]];
        pos -> pieces[SQ120(sq)] = tp;
    }

	pos -> side = tempSide;
    pos -> castlePerm = tempCastlePerm;
    pos -> enPass = tempEnPass;

    pos -> posKey = generatePosKey(pos);

	updateListsMaterial(pos);

    ASSERT(checkBoard(pos));
}