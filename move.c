#include <stdio.h>
#include <string.h>
#include "move.h"
#include "position.h"
#include "bitboard.h"

/*
	There are special moves for kings and pawns only
	For kings, 1 = O-O, 2 = O-O-O
	For pawns, 4 = Q, 3 = R, 2 = B, 1 = N
*/


int adjustCastlingRights(GameState *pos, int src, int dst, int piece)
{
	int castlingRights = pos->castlingRights;
	if (src == a1 || dst == a1)
	{
		castlingRights &= (WHITE_OO | BLACK_OO | BLACK_OOO);
	}

	if (src == h1 || dst == h1)
	{
		castlingRights &= (WHITE_OOO | BLACK_OO | BLACK_OOO);
	}
	
	if (src == a8 || dst == a8)
	{
		castlingRights &= (BLACK_OO | WHITE_OO | WHITE_OOO);
	}
	
	if (src == h8 || dst == h8)
	{
		castlingRights &= (BLACK_OOO | WHITE_OO | WHITE_OOO);
	}
	
	if (piece == K)
	{
		castlingRights &= (BLACK_OO | BLACK_OOO);
	}
	
	if (piece == k)
	{
		castlingRights &= (WHITE_OO | WHITE_OOO);
	}
	return castlingRights;
}

GameState playMove(GameState *pos, Move move, int *isLegal)
{	
	GameState newPos;
	memcpy(&newPos, pos, sizeof(GameState));
	int turn = pos->turn;
	int piece = GET_MOVE_PIECE(move);
	int src = GET_MOVE_SRC(move);
	int dst = GET_MOVE_DST(move);
	int promotion = GET_MOVE_PROMOTION(move);
	int offset = 6 * turn;
	
	// Clear Source
	clear_square(newPos.pieceBitboards[piece], src);
	clear_square(newPos.occupancies[turn], src);
	
	// En Passant Moves
	if ((piece == P || piece == p) && IS_MOVE_EP(move))
	{
		if (turn == WHITE)
		{
			clear_square(newPos.pieceBitboards[p], dst - 8);
			clear_square(newPos.occupancies[BLACK], dst - 8);
		}
		else
		{
			clear_square(newPos.pieceBitboards[P], dst + 8);
			clear_square(newPos.occupancies[WHITE], dst + 8);
		}
	}
	
	// Clear Destination
	if (turn == WHITE)
	{
		clear_square(newPos.occupancies[BLACK], dst);
		for (int i = p; i <=k; i++)
		{
			clear_square(newPos.pieceBitboards[i], dst);
		}
		
		newPos.turn = BLACK;
	}
	else
	{
		clear_square(newPos.occupancies[WHITE], dst);
		for (int i = P; i <=K; i++)
		{
			clear_square(newPos.pieceBitboards[i], dst);
		}
		
		newPos.turn = WHITE;
		newPos.fullMove += 1;
	}
	
	// Set destination
	// If pawn promotion
	set_square(newPos.occupancies[turn], dst);
	if (piece == (P + offset) && promotion)
	{
		set_square(newPos.pieceBitboards[promotion + offset], dst);
	}
	else
	{
		set_square(newPos.pieceBitboards[piece], dst);
	}
	
	// Castling
	if (piece == (K + offset) && IS_MOVE_CASTLES(move))
	{
		set_square(newPos.pieceBitboards[piece], dst);
		
		//Short Castling
		if (src < dst)
		{
			clear_square(newPos.pieceBitboards[R + offset], dst + 1);
			clear_square(newPos.occupancies[turn], dst + 1);
			set_square(newPos.pieceBitboards[R + offset], dst - 1);
			set_square(newPos.occupancies[turn], dst - 1);
		}
		// Long Castling
		else if (src > dst)
		{
			clear_square(newPos.pieceBitboards[R + offset], dst - 2);
			clear_square(newPos.occupancies[turn], dst - 2);
			set_square(newPos.pieceBitboards[R + offset], dst + 1);
			set_square(newPos.occupancies[turn], dst + 1);
		}
	}
	
	newPos.occupancies[BOTH] = newPos.occupancies[WHITE] | newPos.occupancies[BLACK];
	newPos.castlingRights = (pos->castlingRights) ? adjustCastlingRights(pos, src, dst, piece) : 0;
	
	// Reset 50 move counter if capture or pawn push
	if (GET_MOVE_CAPTURED(move) || piece == P || piece == p)
	{
		newPos.halfMoveClock = 0;
	}
	else
	{
		newPos.halfMoveClock += 1;
	}
	
	newPos.enpassantSquare = none;
	// Might be wrong turn here if perft fails
	if (IS_MOVE_DPP(move))
	{
		newPos.enpassantSquare = src + 8 - (16 * turn);
	}
	
	// Legality Check
	int kingLocation = getFirstBitSquare(newPos.pieceBitboards[K + offset]);
	if (isSquareAttacked(&newPos, kingLocation, newPos.turn) != 0)
	{
		*isLegal = 0;
	}
	else
	{
		*isLegal = 1;
	}
	
	return newPos;
}

void printMove(Move m)
{
	int promotion = GET_MOVE_PROMOTION(m);
	printf("%s%s%s", squareNames[GET_MOVE_SRC(m)], squareNames[GET_MOVE_DST(m)], (m) ? pieceNotation[promotion] : "");
}
