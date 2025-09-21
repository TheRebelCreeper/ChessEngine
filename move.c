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


unsigned char adjustCastlingRights(GameState *pos, int src, int dst, int piece)
{
	unsigned char castlingRights = pos->castlingRights;
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
	U64 hashKey = pos->key;
	
	// Clear Source
	hashKey ^= sideKey;
	hashKey ^= pieceKeys[piece][src];
	clear_square(newPos.pieceBitboards[piece], src);
	clear_square(newPos.occupancies[turn], src);
	
	// En Passant Moves
	if ((piece == P || piece == p) && IS_MOVE_EP(move))
	{
		hashKey ^= epKey[pos->enpassantSquare & 7];
		if (turn == WHITE)
		{
			hashKey ^= pieceKeys[p][dst - 8];
			clear_square(newPos.pieceBitboards[p], dst - 8);
			clear_square(newPos.occupancies[BLACK], dst - 8);
		}
		else
		{
			hashKey ^= pieceKeys[P][dst + 8];
			clear_square(newPos.pieceBitboards[P], dst + 8);
			clear_square(newPos.occupancies[WHITE], dst + 8);
		}
	}
	
	// Clear Destination
	if (turn == WHITE)
	{
		clear_square(newPos.occupancies[BLACK], dst);
		
		int victim = GET_MOVE_CAPTURED(move);
		if (victim != NO_CAPTURE)
		{
			clear_square(newPos.pieceBitboards[victim + 6], dst);
			hashKey ^= pieceKeys[victim + 6][dst];
		}
		
		newPos.turn = BLACK;
	}
	else
	{
		clear_square(newPos.occupancies[WHITE], dst);
		
		int victim = GET_MOVE_CAPTURED(move);
		if (victim != NO_CAPTURE)
		{
			clear_square(newPos.pieceBitboards[victim], dst);
			hashKey ^= pieceKeys[victim][dst];
		}
		
		newPos.turn = WHITE;
		newPos.fullMove += 1;
	}
	
	// Set destination
	// If pawn promotion
	set_square(newPos.occupancies[turn], dst);
	if (piece == (P + offset) && promotion)
	{
		hashKey ^= pieceKeys[promotion + offset][dst];
		set_square(newPos.pieceBitboards[promotion + offset], dst);
	}
	else
	{
		hashKey ^= pieceKeys[piece][dst];
		set_square(newPos.pieceBitboards[piece], dst);
	}
	
	// Castling
	if (piece == (K + offset) && IS_MOVE_CASTLES(move))
	{
		
		//Short Castling
		if (src < dst)
		{
			hashKey ^= pieceKeys[R + offset][dst + 1];
			clear_square(newPos.pieceBitboards[R + offset], dst + 1);
			clear_square(newPos.occupancies[turn], dst + 1);

			hashKey ^= pieceKeys[R + offset][dst - 1];
			set_square(newPos.pieceBitboards[R + offset], dst - 1);
			set_square(newPos.occupancies[turn], dst - 1);
		}
		// Long Castling
		else if (src > dst)
		{
			hashKey ^= pieceKeys[R + offset][dst - 2];
			clear_square(newPos.pieceBitboards[R + offset], dst - 2);
			clear_square(newPos.occupancies[turn], dst - 2);

			hashKey ^= pieceKeys[R + offset][dst + 1];
			set_square(newPos.pieceBitboards[R + offset], dst + 1);
			set_square(newPos.occupancies[turn], dst + 1);
		}
	}
	
	hashKey ^= castleKeys[newPos.castlingRights];
	newPos.occupancies[BOTH] = newPos.occupancies[WHITE] | newPos.occupancies[BLACK];
	newPos.castlingRights = (pos->castlingRights) ? adjustCastlingRights(pos, src, dst, piece) : 0;
	hashKey ^= castleKeys[newPos.castlingRights];
	
	// Reset 50 move counter if capture or pawn push
	if (GET_MOVE_CAPTURED(move) != NO_CAPTURE || piece == P || piece == p)
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
		hashKey ^= epKey[newPos.enpassantSquare & 7];
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

	newPos.key = hashKey;

	return newPos;
}

void printMove(Move m)
{
	int promotion = GET_MOVE_PROMOTION(m);
	if(m)
	{
		printf("%s%s%s", squareNames[GET_MOVE_SRC(m)], squareNames[GET_MOVE_DST(m)], (m) ? pieceNotation[promotion] : "");
	}
	else
	{
		printf("0000");
	}
}
