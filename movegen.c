#include <stdio.h>
#include <stdlib.h>
#include "bitboard.h"
#include "position.h"
#include "movegen.h"

void generateMoves(struct GameState pos)
{
	int src, dst;
	int turn = pos.turn;
	int offset = 6 * turn;
	
	U64 pieceBB, pieceAttacks, friendlyPieces, occupancy;
	
	occupancy = getAllPieces();
	friendlyPieces = (turn == WHITE) ? getWhitePieces() : getBlackPieces();
	
	
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
	
	// Generate King Moves
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
