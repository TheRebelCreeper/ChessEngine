#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitboard.h"
#include "position.h"
#include "movegen.h"
#include "list.h"

//TODO Could possibly speed up by using a fixed size array instead of linked list

void generatePawnMoves(GameState *pos, int turn, int offset, MoveList *moveList)
{
	int i = moveList->nextOpen;
	int src, dst, enpassantSquare;
	U64 pieceBB, pieceAttacks, enemyPieces, occupancy;
	U64 singlePushTarget, doublePushTarget;
	occupancy = pos->occupancies[BOTH];
	enemyPieces = (turn == WHITE) ? pos->occupancies[BLACK] : pos->occupancies[WHITE];

	// Generate pawn moves
	pieceBB = pos->pieceBitboards[P + offset];
	
	// Pawn pushes
	if (turn == WHITE)
	{
		singlePushTarget = (pieceBB << 8) & ~occupancy;
		doublePushTarget = (singlePushTarget << 8) & Rank4 & ~occupancy;
	}
	else
	{
		singlePushTarget = (pieceBB >> 8) & ~occupancy;
		doublePushTarget = (singlePushTarget >> 8) & Rank5 & ~occupancy;
	}
	
	while (singlePushTarget)
	{
		dst = getFirstBitSquare(singlePushTarget);
		src = dst - 8 + (16 * turn);
		if ((dst <= h8 && dst >= a8) || (dst >= a1 && dst <= h1))
		{
			moveList->list[i] = createMove(P + offset, src, dst, Q, none);
			moveList->list[i++].prop |= IS_PROMOTION;
			moveList->list[i] = createMove(P + offset, src, dst, R, none);
			moveList->list[i++].prop |= IS_PROMOTION;
			moveList->list[i] = createMove(P + offset, src, dst, B, none);
			moveList->list[i++].prop |= IS_PROMOTION;
			moveList->list[i] = createMove(P + offset, src, dst, N, none);
			moveList->list[i++].prop |= IS_PROMOTION;
		}
		else
		{
			moveList->list[i++] = createMove(P + offset, src, dst, NO_SPECIAL, none);
		}		
		clear_lsb(singlePushTarget);
	}
	
	while (doublePushTarget)
	{
		dst = getFirstBitSquare(doublePushTarget);
		src = dst - 16 + (32 * turn);
		enpassantSquare = src + 8 - (16 * turn);
		moveList->list[i++] = createMove(P + offset, src, dst, NO_SPECIAL, enpassantSquare);
		clear_lsb(doublePushTarget);
	}
	
	// Pawn Captures
	while (pieceBB)
	{	
		src = getFirstBitSquare(pieceBB);
		pieceAttacks = pawnAttacks[turn][src] & enemyPieces;

		if (pos->enpassantSquare != none)
		{
			U64 epAttacks = pawnAttacks[turn][src] & (1ULL << pos->enpassantSquare);
			if (epAttacks)
			{
				dst = pos->enpassantSquare;
				moveList->list[i] = createMove(P + offset, src, dst, NO_SPECIAL, none);
				moveList->list[i++].prop |= IS_EN_PASSANT;
			}
		}
		
		while (pieceAttacks)
		{
			dst = getFirstBitSquare(pieceAttacks);
			// Promotion
			if ((dst <= h8 && dst >= a8) || (dst >= a1 && dst <= h1))
			{
				moveList->list[i] = createMove(P + offset, src, dst, Q, none);
				moveList->list[i++].prop |= IS_PROMOTION;
				moveList->list[i] = createMove(P + offset, src, dst, R, none);
				moveList->list[i++].prop |= IS_PROMOTION;
				moveList->list[i] = createMove(P + offset, src, dst, B, none);
				moveList->list[i++].prop |= IS_PROMOTION;
				moveList->list[i] = createMove(P + offset, src, dst, N, none);
				moveList->list[i++].prop |= IS_PROMOTION;
			}
			else
			{
				moveList->list[i++] = createMove(P + offset, src, dst, NO_SPECIAL, none);
			}
			clear_lsb(pieceAttacks);
		}
		clear_lsb(pieceBB);
	}
	moveList->nextOpen = i;
}

void generateKingMoves(GameState *pos, int turn, int offset, MoveList *moveList)
{
	int i = moveList->nextOpen;
	int src, dst;
	U64 pieceBB, pieceAttacks, friendlyPieces, occupancy;
	occupancy = pos->occupancies[BOTH];
	friendlyPieces = pos->occupancies[turn];

	pieceBB = pos->pieceBitboards[K + offset];
	int castlingRights = pos->castlingRights;
	src = getFirstBitSquare(pieceBB);
	if (turn == WHITE)
	{
		if (castlingRights & WHITE_OO && !get_square(occupancy, f1) && !get_square(occupancy, g1))
		{
			if (!(isSquareAttacked(pos, e1, BLACK) || isSquareAttacked(pos, f1, BLACK)))
			{
				moveList->list[i++] = createMove(K + offset, src, src + 2, OO_SPECIAL, none);
			}
		}
		if (castlingRights & WHITE_OOO && !get_square(occupancy, b1) && !get_square(occupancy, c1) && !get_square(occupancy, d1))
		{
			if (!(isSquareAttacked(pos, e1, BLACK) || isSquareAttacked(pos, d1, BLACK)))
			{
				moveList->list[i++] = createMove(K + offset, src, src - 2, OOO_SPECIAL, none);
			}
		}
	}
	else
	{
		if (castlingRights & BLACK_OO && !get_square(occupancy, f8) && !get_square(occupancy, g8))
		{
			if (!(isSquareAttacked(pos, e8, WHITE) || isSquareAttacked(pos, f8, WHITE)))
			{
				moveList->list[i++] = createMove(K + offset, src, src + 2, OO_SPECIAL, none);
			}
		}
		if (castlingRights & BLACK_OOO && !get_square(occupancy, b8) && !get_square(occupancy, c8) && !get_square(occupancy, d8))
		{
			if (!(isSquareAttacked(pos, e8, WHITE) || isSquareAttacked(pos, d8, WHITE)))
			{
				moveList->list[i++] = createMove(K + offset, src, src - 2, OOO_SPECIAL, none);
			}
		}
	}
	
	// Generate King Moves
	while (pieceBB)
	{
		src = getFirstBitSquare(pieceBB);
		pieceAttacks = kingAttacks[src] & ~friendlyPieces;
		while (pieceAttacks)
		{
			dst = getFirstBitSquare(pieceAttacks);
			moveList->list[i++] = createMove(K + offset, src, dst, NO_SPECIAL, none);
			clear_lsb(pieceAttacks);
		}
		clear_lsb(pieceBB);
	}
	moveList->nextOpen = i;
}

void generateKnightMoves(GameState *pos, int turn, int offset, MoveList *moveList)
{
	int i = moveList->nextOpen;
	int src, dst;
	
	U64 pieceBB, pieceAttacks, friendlyPieces;
	friendlyPieces = pos->occupancies[turn];
	
	// Generate Knight Moves
	pieceBB = pos->pieceBitboards[N + offset];
	while (pieceBB)
	{
		src = getFirstBitSquare(pieceBB);
		pieceAttacks = knightAttacks[src] & ~friendlyPieces;
		while (pieceAttacks)
		{
			dst = getFirstBitSquare(pieceAttacks);
			moveList->list[i++] = createMove(N + offset, src, dst, NO_SPECIAL, none);
			clear_lsb(pieceAttacks);
		}
		clear_lsb(pieceBB);
	}
	moveList->nextOpen = i;
}

void generateBishopMoves(GameState *pos, int turn, int offset, MoveList *moveList)
{
	int i = moveList->nextOpen;
	int src, dst;
	
	U64 pieceBB, pieceAttacks, friendlyPieces, occupancy;
	occupancy = pos->occupancies[BOTH];
	friendlyPieces = pos->occupancies[turn];
	
	// Generate Bishop Moves
	pieceBB = pos->pieceBitboards[B + offset];
	while (pieceBB)
	{
		src = getFirstBitSquare(pieceBB);
		pieceAttacks = getBishopAttacks(src, occupancy) & ~friendlyPieces;
		while (pieceAttacks)
		{
			dst = getFirstBitSquare(pieceAttacks);
			moveList->list[i++] = createMove(B + offset, src, dst, NO_SPECIAL, none);
			clear_lsb(pieceAttacks);
		}
		clear_lsb(pieceBB);
	}
	moveList->nextOpen = i;
}

void generateRookMoves(GameState *pos, int turn, int offset, MoveList *moveList)
{
	int i = moveList->nextOpen;
	int src, dst;
	
	U64 pieceBB, pieceAttacks, friendlyPieces, occupancy;
	occupancy = pos->occupancies[BOTH];
	friendlyPieces = pos->occupancies[turn];
	
	// Generate Rook Moves
	pieceBB = pos->pieceBitboards[R + offset];
	while (pieceBB)
	{
		src = getFirstBitSquare(pieceBB);
		pieceAttacks = getRookAttacks(src, occupancy) & ~friendlyPieces;
		while (pieceAttacks)
		{
			dst = getFirstBitSquare(pieceAttacks);
			moveList->list[i++] = createMove(R + offset, src, dst, NO_SPECIAL, none);
			clear_lsb(pieceAttacks);
		}
		clear_lsb(pieceBB);
	}
	moveList->nextOpen = i;
}

void generateQueenMoves(GameState *pos, int turn, int offset, MoveList *moveList)
{
	int i = moveList->nextOpen;
	int src, dst;
	
	U64 pieceBB, pieceAttacks, friendlyPieces, occupancy;
	occupancy = pos->occupancies[BOTH];
	friendlyPieces = pos->occupancies[turn];
	
	// Generate Queen Moves
	pieceBB = pos->pieceBitboards[Q + offset];
	while (pieceBB)
	{
		src = getFirstBitSquare(pieceBB);
		pieceAttacks = getQueenAttacks(src, occupancy) & ~friendlyPieces;
		while (pieceAttacks)
		{
			dst = getFirstBitSquare(pieceAttacks);
			moveList->list[i++] = createMove(Q + offset, src, dst, NO_SPECIAL, none);
			clear_lsb(pieceAttacks);
		}
		clear_lsb(pieceBB);
	}
	moveList->nextOpen = i;
}

MoveList generateMoves(GameState *pos, int *size)
{
	MoveList moveList;
	moveList.nextOpen = 0;
	memset(moveList.list, 0, sizeof(Move) * MAX_MOVES);
	GameState tempState;
	int turn = pos->turn;
	int offset = 6 * turn;
	int kingLocation;
	int moveCount = 0;
	
	generatePawnMoves(pos, turn, offset, &moveList);
	generateQueenMoves(pos, turn, offset, &moveList);
	generateKnightMoves(pos, turn, offset, &moveList);
	generateBishopMoves(pos, turn, offset, &moveList);
	generateRookMoves(pos, turn, offset, &moveList);
	generateKingMoves(pos, turn, offset, &moveList);
	
	for (int i = 0; i < moveList.nextOpen; i++)
	{
		Move temp = moveList.list[i];
		//tempState = playMove(pos, temp, &(moveList.list[i].legal));
		//kingLocation = getFirstBitSquare(tempState.pieceBitboards[K + offset]);
		//if (moveList.list[i].legal)
		//{
			moveCount++;
			//moveList.list[i].legal = 1;
			//kingLocation = getFirstBitSquare(tempState.pieceBitboards[k - offset]);
			//if (isSquareAttacked(&tempState, kingLocation, turn) == 1)
			//{
			//	moveList.list[i].prop |= IS_CHECK;
			//}
			if (get_square(pos->occupancies[2], moveList.list[i].dst))
			{
				moveList.list[i].prop |= IS_CAPTURE;
			}
		//}
		//else
		//{
		//	moveList.list[i].legal = 0;
		//}
	}
	*size = moveList.nextOpen;
	return moveList;
}

// Prints psuedolegal moves
void printMoveList(MoveList *list, GameState *pos)
{
	if (list->nextOpen == 0)
	{
		int offset = 6 * pos->turn;
		int kingLocation = getFirstBitSquare(pos->pieceBitboards[K + offset]);
		if (isSquareAttacked(pos, kingLocation, (pos->turn == WHITE) ? BLACK : WHITE) == 0)
		{
			printf("Stalemate\n");
		}
		else
		{
			printf("Checkmate\n");
		}
	}
	else
	{
		for (int i = 0; i < list->nextOpen; i++)
		{
			Move temp = list->list[i];
			printf("%d. ", pos->fullMove);
			if (temp.special == NO_SPECIAL || temp.special == EN_PASSANT_SPECIAL)
			{
				printf("%s%s-%s\n", pieceNotation[temp.piece], squareNames[temp.src], squareNames[temp.dst]);
			}
			else if (temp.piece == K || temp.piece == k)
			{
				printf("%s\n", (temp.special == OO_SPECIAL) ? "O-O" : "O-O-O");
			}
			else
			{
				printf("%s%s-%s=%s\n", pieceNotation[temp.piece], squareNames[temp.src], squareNames[temp.dst], pieceNotation[temp.special]);
			}
		}
	}
}
