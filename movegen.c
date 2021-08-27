#include <stdio.h>
#include <stdlib.h>
#include "bitboard.h"
#include "position.h"
#include "movegen.h"

void generateMoves(struct GameState pos)
{
	int src, dst, enpassantSquare;
	int turn = pos.turn;
	int offset = 6 * turn;
	
	U64 pieceBB, pieceAttacks, friendlyPieces, enemyPieces, occupancy;
	U64 singlePushTarget, doublePushTarget;
	
	occupancy = getAllPieces();
	friendlyPieces = (turn == WHITE) ? getWhitePieces() : getBlackPieces();
	enemyPieces = (turn == WHITE) ? getBlackPieces() : getWhitePieces();
	
	// Generate pawn moves
	pieceBB = pos.pieceBitboards[P + offset];
	
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
		if ((dst < 64 && dst >= 0) && (dst >= a7 || dst <= h1))
		{
			// Promotion
			printf("%d. %s=Q\n", pos.fullMove, squareNames[dst]);
			printf("%d. %s=R\n", pos.fullMove, squareNames[dst]);
			printf("%d. %s=B\n", pos.fullMove, squareNames[dst]);
			printf("%d. %s=N\n", pos.fullMove, squareNames[dst]);
		}
		else
		{
			printf("%d. %s\n", pos.fullMove, squareNames[dst]);
		}
		
		
		clear_square(singlePushTarget, dst);
	}
	
	while (doublePushTarget)
	{
		dst = getFirstBitSquare(doublePushTarget);
		src = dst - 16 + (32 * turn);
		enpassantSquare = src + 8 - (16 * turn);
		//printf("%s\n", squareNames[enpassantSquare]);
		printf("%d. %s\n", pos.fullMove, squareNames[dst]);
		clear_square(doublePushTarget, dst);
	}
	
	// Pawn Captures
	while (pieceBB)
	{
		// TODO take en passant
		
		src = getFirstBitSquare(pieceBB);
		// Perhaps & (enemyPieces | enPassantSquare)
		pieceAttacks = pawnAttacks[turn][src] & enemyPieces;
		
		while (pieceAttacks)
		{
			dst = getFirstBitSquare(pieceAttacks);
			// Promotion
			if ((dst < 64 && dst >= 0) && (dst >= a7 || dst <= h1))
			{
				
				printf("%d. %sx%s=Q\n", pos.fullMove, squareNames[src], squareNames[dst]);
				printf("%d. %sx%s=R\n", pos.fullMove, squareNames[src], squareNames[dst]);
				printf("%d. %sx%s=B\n", pos.fullMove, squareNames[src], squareNames[dst]);
				printf("%d. %sx%s=N\n", pos.fullMove, squareNames[src], squareNames[dst]);
			}
			else
			{
				printf("%d. %sx%s\n", pos.fullMove, squareNames[src], squareNames[dst]);
			}
			clear_square(pieceAttacks, dst);
		}
		clear_square(pieceBB, src);
	}
	
	// TODO Generate castling
	
	// Generate King Moves
	pieceBB = pos.pieceBitboards[K + offset];
	while (pieceBB)
	{
		src = getFirstBitSquare(pieceBB);
		pieceAttacks = kingAttacks[src] & ~friendlyPieces;
		while (pieceAttacks)
		{
			dst = getFirstBitSquare(pieceAttacks);
			clear_square(pieceAttacks, dst);
			printf("%d. K%s\n", pos.fullMove, squareNames[dst]);
		}
		clear_square(pieceBB, src);
	}
	
	// Generate Knight Moves
	pieceBB = pos.pieceBitboards[N + offset];
	while (pieceBB)
	{
		src = getFirstBitSquare(pieceBB);
		pieceAttacks = knightAttacks[src] & ~friendlyPieces;
		while (pieceAttacks)
		{
			dst = getFirstBitSquare(pieceAttacks);
			clear_square(pieceAttacks, dst);
			printf("%d. N%s\n", pos.fullMove, squareNames[dst]);
		}
		clear_square(pieceBB, src);
	}
	
	// Generate Bishop Moves
	pieceBB = pos.pieceBitboards[B + offset];
	while (pieceBB)
	{
		src = getFirstBitSquare(pieceBB);
		pieceAttacks = getBishopAttacks(src, occupancy) & ~friendlyPieces;
		while (pieceAttacks)
		{
			dst = getFirstBitSquare(pieceAttacks);
			clear_square(pieceAttacks, dst);
			printf("%d. B%s\n", pos.fullMove, squareNames[dst]);
		}
		clear_square(pieceBB, src);
	}
	
	// Generate Rook Moves
	pieceBB = pos.pieceBitboards[R + offset];
	while (pieceBB)
	{
		src = getFirstBitSquare(pieceBB);
		pieceAttacks = getRookAttacks(src, occupancy) & ~friendlyPieces;
		while (pieceAttacks)
		{
			dst = getFirstBitSquare(pieceAttacks);
			clear_square(pieceAttacks, dst);
			printf("%d. R%s\n", pos.fullMove, squareNames[dst]);
		}
		clear_square(pieceBB, src);
	}
	
	// Generate Queen Moves
	pieceBB = pos.pieceBitboards[Q + offset];
	while (pieceBB)
	{
		src = getFirstBitSquare(pieceBB);
		pieceAttacks = getQueenAttacks(src, occupancy) & ~friendlyPieces;
		while (pieceAttacks)
		{
			dst = getFirstBitSquare(pieceAttacks);
			clear_square(pieceAttacks, dst);
			printf("%d. Q%s\n", pos.fullMove, squareNames[dst]);
		}
		clear_square(pieceBB, src);
	}
	
}
