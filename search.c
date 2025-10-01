#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "search.h"
#include "move.h"
#include "movegen.h"
#include "tt.h"
#include "util.h"

//static U64 deltaPruneCount = 0;
//static U64 deltaPruneTotal = 0;
int followingPV = 0;

void checkTimeLeft(SearchInfo *info)
{
    // .. check if time up, or interrupt from GUI
    if (info->timeset == 1 && GetTimeMs() > info->stoptime) {
        info->stopped = 1;
    }
    ReadInput(info);
}

void scoreMoves(MoveList *moves, GameState *pos, Move ttMove, SearchInfo *info, int use_see)
{
    int i;
    int ply = info->ply;

    for (i = 0; i < moves->nextOpen; i++) {
        // Score TT hits
        if (moves->list[i] == ttMove) {
            moves->score[i] = TT_HIT_SCORE;
            continue;
        }

        // Score captures
        if (moves->list[i] & IS_CAPTURE) {
            int offset = 6 * pos->turn;
            moves->score[i] = MVV_LVA_TABLE[GET_MOVE_PIECE(moves->list[i]) - offset][GET_MOVE_CAPTURED(moves->list[i])]
                              + KILLER_ONE;

            // Give bad score to results with negative SEE
            if (use_see && see(pos, GET_MOVE_DST(moves->list[i])) < 0) {
                moves->score[i] -= KILLER_ONE;
            }
        }
        // Score quiet moves
        else {
            // Check First Killer Move
            if (moves->list[i] == info->killerMoves[0][ply]) {
                moves->score[i] = KILLER_ONE;
            }
            // Check Second Killer Move
            else if (moves->list[i] == info->killerMoves[1][ply]) {
                moves->score[i] = KILLER_TWO;
            }
            else {
                moves->score[i] = MIN(
                    info->history[pos->turn][GET_MOVE_SRC(moves->list[i])][GET_MOVE_DST(moves->list[i])] +
                    HISTORY_SCORE_MIN,
                    HISTORY_SCORE_MAX);
            }
        }
    }
}

void pickMove(MoveList *moves, int startIndex)
{
    int bestIndex = startIndex;
    int i;
    for (i = startIndex; i < moves->nextOpen; i++) {
        if (moves->score[i] > moves->score[bestIndex])
            bestIndex = i;
    }

    Move temp = moves->list[startIndex];
    int tempScore = moves->score[startIndex];
    moves->list[startIndex] = moves->list[bestIndex];
    moves->score[startIndex] = moves->score[bestIndex];
    moves->list[bestIndex] = temp;
    moves->score[bestIndex] = tempScore;
}

inline int is_repetition(GameState *pos)
{
    int reps = 0;
    for (int i = 1; i < pos->halfMoveClock; i++) {
        int idx = historyIndex - i;
        if (idx < 0)
            break;
        if (posHistory[idx] == pos->key)
            reps++;
    }
    //return reps >= 2; //This handles true threefold instead of single rep
    return reps != 0; // Detects a single rep
}

inline int isTactical(Move move, int inCheck, int givesCheck)
{
    return GET_MOVE_CAPTURED(move) != NO_CAPTURE || GET_MOVE_PROMOTION(move) != NO_PROMOTION || inCheck || givesCheck;
}

inline int okToReduce(Move move, int inCheck, int givesCheck, int pv)
{
    return !isTactical(move, inCheck, givesCheck) && !pv;
}

int negaMax(int alpha, int beta, int depth, GameState *pos, SearchInfo *info, int pruneNull)
{
    char nodeBound = TT_ALL;
    int size, i, legal, moveCount = 0;
    int ply = info->ply;
    int inCheck = isInCheck(pos);
    int isPVNode = beta - alpha > 1;
    int isRoot = (ply == 0);
    int enableFutilityPruning = 0;
    int staticEval = evaluation(pos);

    MoveList moveList;
    Move bestMove = 0;
    int bestScore = -INF;

    info->pvTableLength[ply] = depth;
    info->nodes++;

    // Search has exceeded max depth, return static eval
    if (ply > MAX_PLY) {
        return staticEval;
    }

    // Update time left
    if ((info->nodes & 2047) == 0) {
        checkTimeLeft(info);
        // Ran out of time
        if (info->stopped)
            return alpha;
    }

    // Increase depth if currently in check since there are few replies
    if (inCheck)
        depth++;

    // Enter quiescence if not in check
    if (depth <= 0) {
        info->nodes--;
        return quiescence(alpha, beta, depth, pos, info);
    }

    // Mate distance pruning
    // Stops from searching for mate when faster mate already found
    if (!isRoot) {
        int distance = INF - ply;
        if (distance < beta) {
            beta = distance;
            if (alpha >= distance)
                return distance;
        }

        distance = -INF + ply;
        if (distance > alpha) {
            alpha = distance;
            if (beta <= distance)
                return distance;
        }
    }

    // Search for draws and repetitions
    // Don't need to search for repetition if halfMoveClock is low
    if (!isRoot && ((pos->halfMoveClock > 4 && is_repetition(pos)) || pos->halfMoveClock == 100 ||
                    insufficientMaterial(pos))) {
        // Cut the pv line if this is a draw
        info->pvTableLength[ply] = 0;
        return 0;
    }

    // Check hash table for best move
    // Do not cut if pv node
    Move ttMove = 0;
    int score = probeTT(pos, &ttMove, alpha, beta, depth, ply);
    if (score != INVALID_SCORE && !isRoot && !isPVNode) {
        return score;
    }

    // Static Null Move Pruning / Reverse Futility Pruning
    // TODO test out this version
    // if (depth < 3 && !isPVNode && pruneNull && !inCheck && abs(beta) < CHECKMATE && !onlyHasPawns(pos, pos->turn))
    if (depth < 3 && !isPVNode && !inCheck && abs(beta) < CHECKMATE && !onlyHasPawns(pos, pos->turn)) {
        // Try margin of 180 after working on TT-bug
        int evalMargin = 120 * depth;
        if (staticEval - evalMargin >= beta)
            return staticEval - evalMargin;
    }

    // Razoring
    // If static eval is a good amount below alpha, we are probably at an all-node.
    // Do a qsearch just to confirm. If the qsearch fails high, a capture gained back
    // the material and trust its result since a quiet move probably can't gain
    // as much.
    if (!isPVNode && !inCheck
        && abs(alpha) < CHECKMATE
        && depth <= 3 && staticEval <= alpha - RAZOR_MARGIN[depth]) {
        if (depth == 1)
            return quiescence(alpha, beta, 0, pos, info);

        int rWindow = alpha - RAZOR_MARGIN[depth];
        int value = quiescence(rWindow, rWindow + 1, 0, pos, info);
        // Fail hard here to be safe
        if (value <= rWindow)
            return value;
    }

    // Null move pruning
    if (pruneNull && staticEval >= beta && !isPVNode && !inCheck && depth >= 3 && !onlyHasPawns(pos, pos->turn)) {
        // Reduce by either 3 or 4 ply depending on depth
        int r = (depth <= 6) ? 3 : 4;

        // Make the null move
        GameState newPos;
        memcpy(&newPos, pos, sizeof(GameState));
        newPos.turn ^= 1;
        newPos.enpassantSquare = none;
        newPos.key ^= sideKey;
        if (pos->enpassantSquare != none)
            newPos.key ^= epKey[pos->enpassantSquare & 7];
        info->ply++;

        // Search resulting position with reduced depth
        score = -negaMax(-beta, -beta + 1, depth - 1 - r, &newPos, info, 0);

        // Unmake null move
        info->ply--;

        // Ran out of time
        if (info->stopped)
            return bestScore;

        if (score >= beta && abs(score) < CHECKMATE)
            return score;
    }

    // Check if move is eligible for futility pruning
    if (depth < 9 && !isPVNode && !inCheck && alpha < CHECKMATE) {
        if (staticEval + futilityMargins[depth] <= alpha)
            enableFutilityPruning = 1;
    }

    moveList = generateMoves(pos, &size);
    scoreMoves(&moveList, pos, ttMove, info, 1);

    for (i = 0; i < size; i++) {
        pickMove(&moveList, i);
        Move current = moveList.list[i];
        GameState newState = playMove(pos, current, &legal);

        // Increment history index to next depth

        if (!legal)
            continue;

        moveCount++;
        info->ply++;

        // Save the current move into history
        historyIndex++;
        posHistory[historyIndex] = newState.key;

        // Does this move give check
        int givesCheck = isInCheck(&newState);

        // Futility Pruning
        if (enableFutilityPruning && moveCount > 1) {
            if (!isTactical(current, inCheck, givesCheck)) {
                info->ply--;
                historyIndex--;
                continue;
            }
        }

        // LMR psuedocode comes from https://web.archive.org/web/20150212051846/http://www.glaurungchess.com/lmr.html
        // via Tord Romstad

        // Do a full depth search on PV
        if (moveCount == 1) {
            score = -negaMax(-beta, -alpha, depth - 1, &newState, info, 1);
        }
        // LMR on non PV node
        else {
            int r = 0.99 + log(depth) * log(moveCount) / 3.14;
            if (moveCount > FULL_DEPTH_MOVES && (depth - r - 1 > 0) && okToReduce(
                    current, inCheck, givesCheck, isPVNode)) {
                // Reduced search without null moves
                score = -negaMax(-alpha - 1, -alpha, depth - r - 1, &newState, info, 1);
            }
            else {
                score = alpha + 1;
            }

            if (score > alpha) {
                score = -negaMax(-alpha - 1, -alpha, depth - 1, &newState, info, 1);
                if ((score > alpha) && (score < beta)) {
                    score = -negaMax(-beta, -alpha, depth - 1, &newState, info, 1);
                }
            }
        }

        // Update bestMove whenever found so all-nodes can be stored in TT
        if (score > bestScore) {
            bestScore = score;
            bestMove = current;
        }

        // Unmake move by removing current move from history
        info->ply--;
        historyIndex--;

        if (info->stopped)
            return (bestScore == -INF) ? alpha : bestScore;

        if (score >= beta) {
            saveTT(pos, current, score, TT_CUT, depth, ply);
            // If the move is not a capture, save as killer move
            if ((current & IS_CAPTURE) == 0) {
                if (current != info->killerMoves[0][ply]) {
                    info->killerMoves[1][ply] = info->killerMoves[0][ply];
                    info->killerMoves[0][ply] = current;
                }
                info->history[pos->turn][GET_MOVE_SRC(current)][GET_MOVE_DST(current)] += (depth * depth);
                if (info->history[pos->turn][GET_MOVE_SRC(current)][GET_MOVE_DST(current)] > HISTORY_SCORE_MAX) {
                    info->history[pos->turn][GET_MOVE_SRC(current)][GET_MOVE_DST(current)] >>= 1;
                }
            }
            return score;
        }

        if (score > alpha) {
            nodeBound = TT_PV;
            info->pvTable[ply][ply] = current;
            if (depth > 1) {
                // Crazy memcpy which copies PV from lower depth to current depth
                memcpy((info->pvTable[ply]) + ply + 1, (info->pvTable[ply + 1]) + ply + 1,
                       info->pvTableLength[ply + 1] * sizeof(Move));
                info->pvTableLength[ply] = info->pvTableLength[ply + 1] + 1;
            }
            alpha = score;
        }
    }

    // If only one legal move make the move right away
    if (moveCount == 1 && isRoot && info->timeset == 1 && depth > 2) {
        info->stopped = 1;
    }

    if (moveCount == 0) {
        // When no more legal moves, the pv does not exist
        info->pvTableLength[ply] = 0;
        if (inCheck) {
            return -INF + ply;
        }
        return 0;
    }

    saveTT(pos, bestMove, bestScore, nodeBound, depth, ply);
    return bestScore;
}

// Cannot enter while in check initially
int quiescence(int alpha, int beta, int depth, GameState *pos, SearchInfo *info)
{
    MoveList moveList;
    int size, i, legal, moveCount = 0;
    int inCheck = isInCheck(pos);

    info->nodes++;

    int staticEval = evaluation(pos);
    int bestScore = staticEval;

    // Check if time is up every 2048 nodes
    if ((info->nodes & 2047) == 0) {
        checkTimeLeft(info);
        if (info->stopped)
            return staticEval;
    }

    if (info->ply > MAX_PLY) {
        return staticEval;
    }

    if (staticEval >= beta && !inCheck) {
        return staticEval;
    }

    if (staticEval > alpha && !inCheck) {
        alpha = staticEval;
    }

    moveList = generateMoves(pos, &size);
    scoreMoves(&moveList, pos, 0, info, 1);

    // TODO: use moves that give check within the first two ply of qsearch
    for (i = 0; i < size; i++) {
        pickMove(&moveList, i);
        Move current = moveList.list[i];

        if (!inCheck && GET_MOVE_CAPTURED(current) == NO_CAPTURE && GET_MOVE_PROMOTION(current) == NO_PROMOTION) {
            continue;
        }

        // Prune captures with bad SEE when not in check
        if (!inCheck && see(pos, GET_MOVE_DST(current)) < 0) {
            continue;
        }

        GameState newState = playMove(pos, current, &legal);
        if (!legal)
            continue;
        moveCount++;

        info->ply++;
        int score = -quiescence(-beta, -alpha, depth, &newState, info);
        info->ply--;

        if (score > bestScore) {
            bestScore = score;
        }

        if (info->stopped)
            return bestScore;

        // Should this return best eval found or beta?
        if (score >= beta) {
            return score;
        }
        if (score > alpha) {
            alpha = score;
        }
    }

    // If no more legal moves and we are in check checkmate
    if (moveCount == 0 && inCheck) {
        return -INF + info->ply;
    }

    // Draw by 50 move rule
    if (pos->halfMoveClock == 100 || insufficientMaterial(pos)) {
        return 0;
    }

    return bestScore;
}

void search(GameState *pos, SearchInfo *rootInfo)
{
    int score, bestScore;
    Move bestMove = 0;
    int alpha = -INF;
    int beta = INF;
    int searchDepth = rootInfo->depth;
    unsigned int start, finish;

    // Clear information for rootInfo. Will have to do this for ID upon each depth
    start = GetTimeMs();
    rootInfo->nodes = 0ULL;
    rootInfo->ply = 0;
    memset(rootInfo->killerMoves, 0, sizeof(rootInfo->killerMoves));
    memset(rootInfo->history, 0, sizeof(rootInfo->history));
    memset(rootInfo->pvTable, 0, sizeof(rootInfo->pvTable));
    memset(rootInfo->pvTableLength, 0, sizeof(rootInfo->pvTableLength));

    for (int ID = 1; ID <= searchDepth; ID++) {
        rootInfo->depth = ID;
        followingPV = 1;

        score = negaMax(alpha, beta, ID, pos, rootInfo, 1);

        if (rootInfo->stopped == 1 && ID > 1) {
            break;
        }
        bestScore = score;

#ifdef ASPIRATION_WINDOW
        if (bestScore <= alpha || bestScore >= beta) {
            alpha = -INF;
            beta = INF;
            ID--;
            continue;
        }

        alpha = MAX(bestScore - 50, -INF);
        beta = MIN(bestScore + 50, INF);
#endif

        bestMove = rootInfo->pvTable[0][0];

        // After searching all possible moves, compile stats
        finish = GetTimeMs() + 1;
        rootInfo->ms = finish - start;
        rootInfo->nps = (unsigned int) (1000 * rootInfo->nodes / (rootInfo->ms));
        rootInfo->bestScore = bestScore;

        int mated = 0;
        if (rootInfo->bestScore > CHECKMATE) {
            rootInfo->bestScore = (INF - rootInfo->bestScore) / 2 + 1;
            mated = 1;
        }
        else if (rootInfo->bestScore < -CHECKMATE) {
            rootInfo->bestScore = (-INF - rootInfo->bestScore) / 2;
            mated = 1;
        }

        printf("info depth %d ", ID);
        printf("score %s %d ", (mated) ? "mate" : "cp", rootInfo->bestScore);
        printf("time %u ", rootInfo->ms);
        printf("nodes %llu ", rootInfo->nodes);
        printf("nps %u ", rootInfo->nps);
        printf("pv");
        for (int i = 0; i < rootInfo->pvTableLength[0]; i++) {
            if (i >= ID || rootInfo->pvTable[0][i] == 0)
                break;
            printf(" ");
            printMove((rootInfo->pvTable[0][i]));
        }
        printf("\n");
    }
    // Print the best move
    printf("bestmove ");
    printMove(bestMove);
    printf("\n");
    //printf("Delta pruning: %llu pruned out of %llu captures (%.2f%%)\n",
    //   deltaPruneCount, deltaPruneTotal,
    //   (deltaPruneTotal > 0) ? (100.0 * deltaPruneCount / deltaPruneTotal) : 0.0);
    //deltaPruneCount = 0;
    //deltaPruneTotal = 0;
}
