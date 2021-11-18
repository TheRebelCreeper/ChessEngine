#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitboard.h"
#include "position.h"
#include "movegen.h"
#include "list.h"

void generatePawnMoves(GameState *pos, int turn, int offset, MoveList *moveList)
{
	int i = moveList->nextOpen;
	int src, dst, piece = P + offset;
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
	
	while (doublePushTarget)
	{
		dst = getFirstBitSquare(doublePushTarget);
		src = dst - 16 + (32 * turn);
		moveList->list[i++] = CREATE_MOVE(src, dst, piece, 0, 0, 1, 0, 0);
		clear_lsb(doublePushTarget);
	}
	
	while (singlePushTarget)
	{
		dst = getFirstBitSquare(singlePushTarget);
		src = dst - 8 + (16 * turn);
		if ((dst <= h8 && dst >= a8) || (dst >= a1 && dst <= h1))
		{
			moveList->list[i++] = CREATE_MOVE(src, dst, piece, Q, 0, 0, 0, 0);
			moveList->list[i++] = CREATE_MOVE(src, dst, piece, R, 0, 0, 0, 0);
			moveList->list[i++] = CREATE_MOVE(src, dst, piece, B, 0, 0, 0, 0);
			moveList->list[i++] = CREATE_MOVE(src, dst, piece, N, 0, 0, 0, 0);
		}
		else
		{
			moveList->list[i++] = CREATE_MOVE(src, dst, piece, 0, 0, 0, 0, 0);
		}		
		clear_lsb(singlePushTarget);
	}
	
	// Pawn Captures
	while (pieceBB)
	{	
		src = getFirstBitSquare(pieceBB);
		pieceAttacks = pawnAttacks[turn][src] & enemyPieces;

		// EP captures
		if (pos->enpassantSquare != none)
		{
			U64 epAttacks = pawnAttacks[turn][src] & (1ULL << pos->enpassantSquare);
			if (epAttacks)
			{
				dst = pos->enpassantSquare;
				moveList->list[i++] = CREATE_MOVE(src, dst, piece, 0, 1, 0, 1, 0);
			}
		}
		
		while (pieceAttacks)
		{
			dst = getFirstBitSquare(pieceAttacks);
			// Promotion
			if ((dst <= h8 && dst >= a8) || (dst >= a1 && dst <= h1))
			{
				moveList->list[i++] = CREATE_MOVE(src, dst, piece, Q, 1, 0, 0, 0);
				moveList->list[i++] = CREATE_MOVE(src, dst, piece, R, 1, 0, 0, 0);
				moveList->list[i++] = CREATE_MOVE(src, dst, piece, B, 1, 0, 0, 0);
				moveList->list[i++] = CREATE_MOVE(src, dst, piece, N, 1, 0, 0, 0);
			}
			else
			{
				moveList->list[i++] = CREATE_MOVE(src, dst, piece, 0, 1, 0, 0, 0);
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
	int src, dst, piece = K + offset;
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
				moveList->list[i++] = CREATE_MOVE(src, src + 2, K, 0, 0, 0, 0, 1);
			}
		}
		if (castlingRights & WHITE_OOO && !get_square(occupancy, b1) && !get_square(occupancy, c1) && !get_square(occupancy, d1))
		{
			if (!(isSquareAttacked(pos, e1, BLACK) || isSquareAttacked(pos, d1, BLACK)))
			{
				moveList->list[i++] = CREATE_MOVE(src, src - 2, K, 0, 0, 0, 0, 1);
			}
		}
	}
	else
	{
		if (castlingRights & BLACK_OO && !get_square(occupancy, f8) && !get_square(occupancy, g8))
		{
			if (!(isSquareAttacked(pos, e8, WHITE) || isSquareAttacked(pos, f8, WHITE)))
			{
				moveList->list[i++] = CREATE_MOVE(src, src + 2, k, 0, 0, 0, 0, 1);
			}
		}
		if (castlingRights & BLACK_OOO && !get_square(occupancy, b8) && !get_square(occupancy, c8) && !get_square(occupancy, d8))
		{
			if (!(isSquareAttacked(pos, e8, WHITE) || isSquareAttacked(pos, d8, WHITE)))
			{
				moveList->list[i++] = CREATE_MOVE(src, src - 2, k, 0, 0, 0, 0, 1);
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
			moveList->list[i++] = CREATE_MOVE(src, dst, piece, 0, 0, 0, 0, 0);
			clear_lsb(pieceAttacks);
		}
		clear_lsb(pieceBB);
	}
	moveList->nextOpen = i;
}

void generateKnightMoves(GameState *pos, int turn, int offset, MoveList *moveList)
{
	int i = moveList->nextOpen;
	int src, dst, piece = N + offset;
	
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
			moveList->list[i++] = CREATE_MOVE(src, dst, piece, 0, 0, 0, 0, 0);
			clear_lsb(pieceAttacks);
		}
		clear_lsb(pieceBB);
	}
	moveList->nextOpen = i;
}

void generateBishopMoves(GameState *pos, int turn, int offset, MoveList *moveList)
{
	int i = moveList->nextOpen;
	int src, dst, piece = B + offset;
	
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
			moveList->list[i++] = CREATE_MOVE(src, dst, piece, 0, 0, 0, 0, 0);
			clear_lsb(pieceAttacks);
		}
		clear_lsb(pieceBB);
	}
	moveList->nextOpen = i;
}

void generateRookMoves(GameState *pos, int turn, int offset, MoveList *moveList)
{
	int i = moveList->nextOpen;
	int src, dst, piece = R + offset;
	
	U64 pieceBB, pieceAttacks, friendlyPieces, occupancy;
	occupancy = pos->occupancies[BOTH];
	friendlyPieces = pos->occupancies[turn];
	
	// Generate Rook Moves
	pieceBB = pos->pieceBitboards[piece];
	while (pieceBB)
	{
		src = getFirstBitSquare(pieceBB);
		pieceAttacks = getRookAttacks(src, occupancy) & ~friendlyPieces;
		while (pieceAttacks)
		{
			dst = getFirstBitSquare(pieceAttacks);
			moveList->list[i++] = CREATE_MOVE(src, dst, piece, 0, 0, 0, 0, 0);
			clear_lsb(pieceAttacks);
		}
		clear_lsb(pieceBB);
	}
	moveList->nextOpen = i;
}

void generateQueenMoves(GameState *pos, int turn, int offset, MoveList *moveList)
{
	int i = moveList->nextOpen;
	int src, dst, piece = Q + offset;
	
	U64 pieceBB, pieceAttacks, friendlyPieces, occupancy;
	occupancy = pos->occupancies[BOTH];
	friendlyPieces = pos->occupancies[turn];
	
	// Generate Queen Moves
	pieceBB = pos->pieceBitboards[piece];
	while (pieceBB)
	{
		src = getFirstBitSquare(pieceBB);
		pieceAttacks = getQueenAttacks(src, occupancy) & ~friendlyPieces;
		while (pieceAttacks)
		{
			dst = getFirstBitSquare(pieceAttacks);
			moveList->list[i++] = CREATE_MOVE(src, dst, piece, 0, 0, 0, 0, 0);
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
	int turn = pos->turn;
	int offset = 6 * turn;
	
	generatePawnMoves(pos, turn, offset, &moveList);
	generateKnightMoves(pos, turn, offset, &moveList);
	generateBishopMoves(pos, turn, offset, &moveList);
	generateRookMoves(pos, turn, offset, &moveList);
	generateKingMoves(pos, turn, offset, &moveList);
	generateQueenMoves(pos, turn, offset, &moveList);
	
	for (int i = 0; i < moveList.nextOpen; i++)
	{
		if (get_square(pos->occupancies[2], GET_MOVE_DST(moveList.list[i])))
		{
			moveList.list[i] |= IS_CAPTURE;
		}
	}
	*size = moveList.nextOpen;
	return moveList;
}
