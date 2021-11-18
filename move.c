#include <stdio.h>
#include <string.h>
#include "move.h"
#include "position.h"
#include "bitboard.h"

int moveEquality(Move m1, Move m2)
{
	return m1 == m2;
}

int compareMoves(const void * a, const void * b)
{
	Move *m1 = (Move*)a;
	Move *m2 = (Move*)b;
	
	return *m2 - *m1;
}

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
	int piece = GET_MOVE_PIECE(move);
	int src = GET_MOVE_SRC(move);
	int dst = GET_MOVE_DST(move);
	int offset = 6 * pos->turn;
	
	// Clear Source
	clear_square(newPos.pieceBitboards[piece], src);
	clear_square(newPos.occupancies[pos->turn], src);
	
	// En Passant Moves
	if ((piece == P || piece == p) && move & IS_EN_PASSANT)
	{
		if (pos->turn == WHITE)
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
	if (pos->turn == WHITE)
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
	set_square(newPos.occupancies[pos->turn], dst);
	if (piece == (P + offset) && move & IS_PROMOTION)
	{
		set_square(newPos.pieceBitboards[GET_MOVE_SPECIAL(move) + offset], dst);
	}
	else
	{
		set_square(newPos.pieceBitboards[piece], dst);
	}
	
	// Castling
	if (piece == (K + offset) && GET_MOVE_SPECIAL(move) != NO_SPECIAL)
	{
		set_square(newPos.pieceBitboards[piece], dst);
		if (GET_MOVE_SPECIAL(move) == OO_SPECIAL)
		{
			clear_square(newPos.pieceBitboards[R + offset], dst + 1);
			clear_square(newPos.occupancies[pos->turn], dst + 1);
			set_square(newPos.pieceBitboards[R + offset], dst - 1);
			set_square(newPos.occupancies[pos->turn], dst - 1);
		}
		else if (GET_MOVE_SPECIAL(move) == OOO_SPECIAL)
		{
			clear_square(newPos.pieceBitboards[R + offset], dst - 2);
			clear_square(newPos.occupancies[pos->turn], dst - 2);
			set_square(newPos.pieceBitboards[R + offset], dst + 1);
			set_square(newPos.occupancies[pos->turn], dst + 1);
		}
	}
	
	newPos.occupancies[BOTH] = newPos.occupancies[WHITE] | newPos.occupancies[BLACK];
	newPos.castlingRights = (pos->castlingRights) ? adjustCastlingRights(pos, src, dst, piece) : 0;
	
	// Reset 50 move counter if capture or pawn push
	if ((move & IS_CAPTURE) || piece == P || piece == p)
	{
		newPos.halfMoveClock = 0;
	}
	else
	{
		newPos.halfMoveClock += 1;
	}
	newPos.enpassantSquare = GET_MOVE_EP_SQUARE(move);
	
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
	printf("%s%s%s", squareNames[GET_MOVE_SRC(m)], squareNames[GET_MOVE_DST(m)], (m & IS_PROMOTION) ? pieceNotation[GET_MOVE_SPECIAL(m)] : "");
}
