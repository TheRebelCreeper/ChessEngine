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
	For pawns, 5 = Q, 4 = R, 3 = B, 2 = N, 1 = EP
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
	//TODO Castling Rights
	
	GameState newPos;
	memcpy(&newPos, &pos, sizeof(GameState));
	int piece = move.piece;
	int src = move.src;
	int dst = move.dst;
	
	if (pos.turn == WHITE)
	{
		for (int i = p; i <=k; i++)
		{
			clear_square(newPos.pieceBitboards[i], dst);
		}
		newPos.turn = BLACK;
	}
	else
	{
		for (int i = P; i <=K; i++)
		{
			clear_square(newPos.pieceBitboards[i], dst);
		}
		newPos.turn = WHITE;
		newPos.fullMove += 1;
	}
	
	clear_square(newPos.pieceBitboards[piece], src);
	set_square(newPos.pieceBitboards[piece], dst);
	setOccupancies(&newPos);
	newPos.halfMoveClock += 1;
	newPos.enpassantSquare = move.epSquare;
	
	
	return newPos;
}
