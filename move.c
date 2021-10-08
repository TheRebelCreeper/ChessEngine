#include <stdio.h>
#include <string.h>
#include "move.h"
#include "position.h"
#include "bitboard.h"
int compareMoves(Move m1, Move m2)
{
	    return (m1.piece == m2.piece) && (m1.src == m2.src) && (m1.dst == m2.dst);
}

/*
	There are special moves for kings and pawns only
	For kings, 1 = O-O, 2 = O-O-O
	For pawns, 4 = Q, 3 = R, 2 = B, 1 = N, 5 = EP
*/
Move createMove(int piece, int src, int dst, int special, int epSquare)
{
	Move newMove;
	newMove.piece = piece;
	newMove.src = src;
	newMove.dst = dst;
	newMove.special = special;
	newMove.epSquare = epSquare;
	return newMove;
}

GameState playMove(GameState pos, Move move)
{	
	GameState newPos;
	memcpy(&newPos, &pos, sizeof(GameState));
	int piece = move.piece;
	int src = move.src;
	int dst = move.dst;
	int offset = 6 * pos.turn;
	
	// Clear Destination
	if (pos.turn == WHITE)
	{
		for (int i = p; i <=k; i++)
		{
			clear_square(newPos.pieceBitboards[i], dst);
		}
		if (move.special == EN_PASSANT_SPECIAL)
		{
			clear_square(newPos.pieceBitboards[p], dst - 8);
		}
		newPos.turn = BLACK;
	}
	else
	{
		for (int i = P; i <=K; i++)
		{
			clear_square(newPos.pieceBitboards[i], dst);
		}
		if (move.special == EN_PASSANT_SPECIAL)
		{
			clear_square(newPos.pieceBitboards[P], dst + 8);
		}
		newPos.turn = WHITE;
		newPos.fullMove += 1;
	}
	
	// Clear Source
	// TODO Castling
	clear_square(newPos.pieceBitboards[piece], src);
	
	// Set destination
	// If pawn promotion
	if (piece == (P + offset) && move.special != EN_PASSANT_SPECIAL)
	{
		set_square(newPos.pieceBitboards[move.special + offset], dst);
	}
	// Castling
	else if (piece == (K + offset) && move.special != NO_SPECIAL)
	{
		
	}
	else
	{
		set_square(newPos.pieceBitboards[piece], dst);
	}
	
	setOccupancies(&newPos);
	newPos.halfMoveClock += 1;
	newPos.enpassantSquare = move.epSquare;
	
	return newPos;
}
