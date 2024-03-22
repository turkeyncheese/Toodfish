#include "pst.h"
#include "tune.h"

eval_t PST[NB_COLOR][NB_PIECE][NB_SQUARE];

void __attribute__((constructor)) pst_init(void) {
    for (int color = WHITE; color <= BLACK; color++)
        for (int piece = KNIGHT; piece < NB_PIECE; piece++)
            for (int square = A1; square <= H8; square++) {
                const int rr = relative_rank_of(color, square);
                const int file = file_of(square), file4 = file > FILE_D ? FILE_H - file : file;

                PST[color][piece][square] =
                    piece == PAWN   ? (eval_t){2 * PieceValue[PAWN] - 200, 200}
                    : piece == KING ? (eval_t){0, 0}
                                    : (eval_t){PieceValue[piece], PieceValue[piece]};

                if (piece == KNIGHT) {
                    eval_add(&PST[color][piece][square], KnightPstSeed[file4]);
                    eval_add(&PST[color][piece][square], KnightPstSeed[4 + rr]);
                } else if (piece == BISHOP)
                    eval_add(&PST[color][piece][square], BishopPstSeed[rr][file4]);
                else if (piece == ROOK) {
                    eval_add(&PST[color][piece][square], RookPstSeed[file4]);
                    eval_add(&PST[color][piece][square], RookPstSeed[4 + rr]);
                } else if (piece == QUEEN) {
                    eval_add(&PST[color][piece][square], QueenPstSeed[file4]);
                    eval_add(&PST[color][piece][square], QueenPstSeed[4 + rr]);
                } else if (piece == KING)
                    eval_add(&PST[color][piece][square], KingPstSeed[rr][file4]);
                else if (piece == PAWN && RANK_2 <= rr && rr <= RANK_7)
                    eval_add(&PST[color][piece][square], PawnPstSeed[rr - RANK_2][file4]);

                if (color == BLACK) {
                    PST[color][piece][square].op *= -1;
                    PST[color][piece][square].eg *= -1;
                }
            }
}
