#include <stdio.h>
#include <stdbool.h>
#include "defs.h"

int rootDepth;

static void checkUp(S_SEARCHINFO *info) {
    if (info -> timeset == true && getTimeMs() > info -> stoptime) {
		info -> stopped = true;
	}

	readInput(info);
}

static void pickNextMove(int moveNum, S_MOVELIST *list) {
	S_MOVE temp;

	int bestScore = 0;
	int bestNum = moveNum;

	for (int i = moveNum; i < list -> count; ++i) {
		if (list -> moves[i].score > bestScore) {
			bestScore = list -> moves[i].score;
			bestNum = i;
		}
	}

	ASSERT(moveNum >= 0 && moveNum < list -> count);
	ASSERT(bestNum >= 0 && bestNum < list -> count);
	ASSERT(bestNum >= moveNum);

	temp = list -> moves[moveNum];
	list -> moves[moveNum] = list -> moves[bestNum];
	list -> moves[bestNum] = temp;
}

static int isRepetition(const S_BOARD *pos) {
	for (int i = pos -> hisPly - pos -> fiftyMove; i < pos->hisPly-1; i++) {
		ASSERT(i >= 0 && i < MAXGAMEMOVES);

		if (pos -> posKey == pos -> history[i].posKey) {
			return true;
		}
	}

	return false;
}

static void clearForSearch(S_BOARD *pos, S_SEARCHINFO *info) {
	for (int i = 0; i < 13; i++) {
		for (int j = 0; j < BRD_SQ_NUM; ++j) {
			pos -> searchHistory[i][j] = 0;
		}
	}

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < MAXDEPTH; ++j) {
			pos -> searchKillers[i][j] = 0;
		}
	}

    pos -> hashTable -> overWrite = 0;
	pos -> hashTable -> hit = 0;
	pos -> hashTable -> cut = 0;
    pos -> ply = 0;

	info -> stopped = 0;
	info -> nodes = 0;
	info -> fh = 0;
	info -> fhf = 0;
}

static int quiescence (int alpha, int beta, S_BOARD *pos, S_SEARCHINFO *info) {
    ASSERT(checkBoard(pos));
	ASSERT(beta > alpha);

	if ((info -> nodes & 2047) == 0) {
		checkUp(info);
	}

	info -> nodes++;

	if (isRepetition(pos) || pos -> fiftyMove >= 100) {
		return 0;
	}

	if (pos -> ply > MAXDEPTH - 1) {
		return evalPosition(pos);
	}

	int score = evalPosition(pos);

	ASSERT(score > -INF && score < INF);

	if (score >= beta) {
		return beta;
	}

	if (score > alpha) {
		alpha = score;
	}

	S_MOVELIST list[1];
    generateAllCaps(pos, list);

	int legal = 0;
	score = -INF;

	for (int moveNum = 0; moveNum < list -> count; moveNum++) {

		pickNextMove(moveNum, list);

        if (!makeMove(pos, list -> moves[moveNum].move)) {
            continue;
        }

		legal++;
		score = -quiescence(-beta, -alpha, pos, info);
        takeMove(pos);

		if (info -> stopped == true) {
			return 0;
		}

		if (score > alpha) {
			if (score >= beta) {
				if (legal == 1) {
					info -> fhf++;
				}

				info -> fh++;
				return beta;
			}

			alpha = score;
		}
    }

	ASSERT(alpha >= oldAlpha);

	return alpha;
}

static int alphaBeta(int alpha, int beta, int depth, S_BOARD *pos, S_SEARCHINFO *info, int doNull) {
    ASSERT(checkBoard(pos));
    ASSERT(beta > alpha);
	ASSERT(depth >= 0);

    if (depth <= 0) {
        return quiescence(alpha, beta, pos, info);
    }

    if ((info -> nodes & 2047) == 0) {
		checkUp(info);
	}

    info -> nodes++;

    if ((isRepetition(pos) || pos -> fiftyMove >= 100) && pos -> ply) {
        return 0;
    }

    if (pos -> ply > MAXDEPTH - 1) {
        return evalPosition(pos);
    }

    int inCheck = sqAttacked(pos -> kingSq[pos -> side], pos -> side ^ 1, pos);

    if (inCheck == true) {
		depth++;
	}

    int score = -INF;
    int pvMove = NOMOVE;

    if (probeHashEntry(pos, &pvMove, &score, alpha, beta, depth) == true ) {
		pos -> hashTable -> cut++;
		return score;
	}

    if (doNull && !inCheck && pos -> ply && (pos -> bigPieces[pos -> side] > 0) && depth >= 4) {
		makeNullMove(pos);
		score = -alphaBeta(-beta, -beta + 1, depth - 4, pos, info, false);
		takeNullMove(pos);

		if (info -> stopped == true) {
			return 0;
		}

		if (score >= beta && abs(score) < ISMATE) {
			info -> nullCut++;
			return beta;
		}
	}

    S_MOVELIST list[1];
    generateAllMoves(pos, list);

    int legal = 0;
    int oldAlpha = alpha;
    int bestMove = NOMOVE;
    int bestScore = -INF;
    score = -INF;

    if (pvMove != NOMOVE) {
		for (int moveNum = 0; moveNum < list -> count; moveNum++) {
			if (list -> moves[moveNum].move == pvMove) {
				list -> moves[moveNum].score = 2000000;
				break;
			}
		}
	}

    for (int moveNum = 0; moveNum < list -> count; moveNum++) {
        if (!makeMove(pos, list -> moves[moveNum].move)) {
            continue;
        }

        legal++;
        score = -alphaBeta(-beta, -alpha, depth - 1, pos, info, true);
        takeMove(pos);

        if (info -> stopped == true) {
            return 0;
        }

        if (score > bestScore) {
            bestScore = score;
            bestMove = list -> moves[moveNum].move;
            if (score > alpha) {
                if (score >= beta) {
					if (legal == 1) {
						info -> fhf++;
					}

					info -> fh++;

					if (!(list -> moves[moveNum].move & MFLAGCAP)) {
						pos -> searchKillers[1][pos -> ply] = pos -> searchKillers[0][pos -> ply];
						pos -> searchKillers[0][pos -> ply] = list -> moves[moveNum].move;
					}

					storeHashEntry(pos, bestMove, beta, HFBETA, depth);

					return beta;
				}

				alpha = score;

				if (!(list -> moves[moveNum].move & MFLAGCAP)) {
					pos -> searchHistory[pos -> pieces[FROMSQ(bestMove)]][TOSQ(bestMove)] += depth;
				}
            }
        }
    }

    if (legal == 0) {
        if (inCheck) {
            return -INF + pos -> ply;
        } else {
            return 0;
        }
    }

    ASSERT(alpha >= oldAlpha);

    if (alpha != oldAlpha) {
        storeHashEntry(pos, bestMove, bestScore, HFEXACT, depth);
    } else {
        storeHashEntry(pos, bestMove, alpha, HFALPHA, depth);
    }
    
    return alpha;
}

void searchPosition(S_BOARD *pos, S_SEARCHINFO *info) {
    int bestMove = NOMOVE;
    int bestScore = -INF;
    int pvMoves = 0;

    clearForSearch(pos, info);

    if (engineOptions -> useBook == true) {
		bestMove = getBookMove(pos);
	}

    if (bestMove == NOMOVE) {
		for (int currentDepth = 1; currentDepth <= info -> depth; currentDepth++) {
			rootDepth = currentDepth;
			bestScore = alphaBeta(-INF, INF, currentDepth, pos, info, true);

			if (info -> stopped == true) {
				break;
			}

			pvMoves = getPvLine(currentDepth, pos);
			bestMove = pos -> pvArray[0];

			if (info -> GAME_MODE == UCIMODE) {
				printf("info score cp %d depth %d nodes %ld time %d ",
					bestScore, currentDepth, info -> nodes, getTimeMs() - info -> starttime);
			}
            
            else if (info -> GAME_MODE == XBOARDMODE && info -> POST_THINKING == true) {
				printf("%d %d %d %ld ",
					currentDepth, bestScore, (getTimeMs() - info -> starttime) / 10,info -> nodes);
			}
            
            else if (info -> POST_THINKING == true) {
				printf("score:%d depth:%d nodes:%ld time:%d(ms) ",
					bestScore, currentDepth, info -> nodes, getTimeMs() - info -> starttime);
			}

			if (info -> GAME_MODE == UCIMODE || info -> POST_THINKING == true) {
				pvMoves = getPvLine(currentDepth, pos);
				
                if (!info -> GAME_MODE == XBOARDMODE) {
					printf("pv");
				}

				for (int pvNum = 0; pvNum < pvMoves; ++pvNum) {
					printf(" %s", printMove(pos -> pvArray[pvNum]));
				}

				printf("\n");
			}
		}
	}

	if (info -> GAME_MODE == UCIMODE) {
		printf("bestmove %s\n", printMove(bestMove));
	}
    
    else if (info -> GAME_MODE == XBOARDMODE) {
		printf("move %s\n", printMove(bestMove));
		makeMove(pos, bestMove);
	} else {
		printf("\n\n***!! Toodfish makes move %s !!***\n\n", printMove(bestMove));
		makeMove(pos, bestMove);
		printBoard(pos);
	}
}