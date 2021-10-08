#include <stdio.h>
#include <stdlib.h>
#include "bitboard.h"
#include "position.h"
#include "movegen.h"
#include "list.h"

//TODO Could possibly speed up by using a fixed size array instead of linked list

void generatePawnMoves(GameState pos, int turn, int offset, Node **moveList)
{
	int src, dst, enpassantSquare;
	U64 pieceBB, pieceAttacks, enemyPieces, occupancy;
	U64 singlePushTarget, doublePushTarget;
	occupancy = pos.occupancies[BOTH];
	enemyPieces = (turn == WHITE) ? pos.occupancies[BLACK] : pos.occupancies[WHITE];

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
		if ((dst <= h8 && dst >= a8) || (dst >= a1 && dst <= h1))
		{
			insert(moveList, createMove(P + offset, src, dst, Q, none));
			insert(moveList, createMove(P + offset, src, dst, R, none));
			insert(moveList, createMove(P + offset, src, dst, B, none));
			insert(moveList, createMove(P + offset, src, dst, N, none));
			
		}
		else
		{
			insert(moveList, createMove(P + offset, src, dst, NO_SPECIAL, none));
		}		
		clear_lsb(singlePushTarget);
	}
	
	while (doublePushTarget)
	{
		dst = getFirstBitSquare(doublePushTarget);
		src = dst - 16 + (32 * turn);
		enpassantSquare = src + 8 - (16 * turn);
		insert(moveList, createMove(P + offset, src, dst, NO_SPECIAL, enpassantSquare));
		clear_lsb(doublePushTarget);
	}
	
	// Pawn Captures
	while (pieceBB)
	{	
		src = getFirstBitSquare(pieceBB);
		pieceAttacks = pawnAttacks[turn][src] & enemyPieces;

		if (pos.enpassantSquare != none)
		{
			U64 epAttacks = pawnAttacks[turn][src] & (1ULL << pos.enpassantSquare);
			if (epAttacks)
			{
				dst = pos.enpassantSquare;
				insert(moveList, createMove(P + offset, src, dst, EN_PASSANT_SPECIAL, none));
			}
		}
		
		while (pieceAttacks)
		{
			dst = getFirstBitSquare(pieceAttacks);
			// Promotion
			if ((dst <= h8 && dst >= a8) || (dst >= a1 && dst <= h1))
			{
				insert(moveList, createMove(P + offset, src, dst, Q, none));
				insert(moveList, createMove(P + offset, src, dst, R, none));
				insert(moveList, createMove(P + offset, src, dst, B, none));
				insert(moveList, createMove(P + offset, src, dst, N, none));
			}
			else
			{
				insert(moveList, createMove(P + offset, src, dst, NO_SPECIAL, none));
			}
			clear_lsb(pieceAttacks);
		}
		clear_lsb(pieceBB);
	}
}

void generateKingMoves(GameState pos, int turn, int offset, Node **moveList)
{
	int src, dst;
	U64 pieceBB, pieceAttacks, friendlyPieces, occupancy;
	occupancy = pos.occupancies[BOTH];
	friendlyPieces = pos.occupancies[turn];

	// TODO Not check if king destination is attacked since should always check if king is in check after move
	pieceBB = pos.pieceBitboards[K + offset];
	int castlingRights = pos.castlingRights;
	src = getFirstBitSquare(pieceBB);
	if (turn == WHITE)
	{
		if (castlingRights & WHITE_OO && !get_square(occupancy, f1) && !get_square(occupancy, g1))
		{
			if (!(isSquareAttacked(pos, e1, BLACK) || isSquareAttacked(pos, f1, BLACK)))
			{
				insert(moveList, createMove(K + offset, src, src + 2, OO_SPECIAL, none));
				//printf("%d. O-O\n", pos.fullMove);
			}
		}
		if (castlingRights & WHITE_OOO && !get_square(occupancy, b1) && !get_square(occupancy, c1) && !get_square(occupancy, d1))
		{
			if (!(isSquareAttacked(pos, e1, BLACK) || isSquareAttacked(pos, d1, BLACK)))
			{
				insert(moveList, createMove(K + offset, src, src - 2, OOO_SPECIAL, none));
				//printf("%d. O-O-O\n", pos.fullMove);
			}
		}
	}
	else
	{
		if (castlingRights & BLACK_OO && !get_square(occupancy, f8) && !get_square(occupancy, g8))
		{
			if (!(isSquareAttacked(pos, e8, WHITE) || isSquareAttacked(pos, f8, WHITE)))
			{
				insert(moveList, createMove(K + offset, src, src + 2, OO_SPECIAL, none));
				//printf("%d. O-O\n", pos.fullMove);
			}
		}
		if (castlingRights & BLACK_OOO && !get_square(occupancy, b8) && !get_square(occupancy, c8) && !get_square(occupancy, d8))
		{
			if (!(isSquareAttacked(pos, e8, WHITE) || isSquareAttacked(pos, d8, WHITE)))
			{
				insert(moveList, createMove(K + offset, src, src - 2, OOO_SPECIAL, none));
				//printf("%d. O-O-O\n", pos.fullMove);
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
			insert(moveList, createMove(K + offset, src, dst, NO_SPECIAL, none));
			clear_lsb(pieceAttacks);
			//printf("%d. K%s\n", pos.fullMove, squareNames[dst]);
		}
		clear_lsb(pieceBB);
	}
}

void generateKnightMoves(GameState pos, int turn, int offset, Node **moveList)
{
	int src, dst;
	
	U64 pieceBB, pieceAttacks, friendlyPieces;
	friendlyPieces = pos.occupancies[turn];
	
	// Generate Knight Moves
	pieceBB = pos.pieceBitboards[N + offset];
	while (pieceBB)
	{
		src = getFirstBitSquare(pieceBB);
		pieceAttacks = knightAttacks[src] & ~friendlyPieces;
		while (pieceAttacks)
		{
			dst = getFirstBitSquare(pieceAttacks);
			insert(moveList, createMove(N + offset, src, dst, NO_SPECIAL, none));
			clear_lsb(pieceAttacks);
			//printf("%d. N%s\n", pos.fullMove, squareNames[dst]);
		}
		clear_lsb(pieceBB);
	}
}

void generateBishopMoves(GameState pos, int turn, int offset, Node **moveList)
{
	int src, dst;
	
	U64 pieceBB, pieceAttacks, friendlyPieces, occupancy;
	occupancy = pos.occupancies[BOTH];
	friendlyPieces = pos.occupancies[turn];
	
	// Generate Bishop Moves
	pieceBB = pos.pieceBitboards[B + offset];
	while (pieceBB)
	{
		src = getFirstBitSquare(pieceBB);
		pieceAttacks = getBishopAttacks(src, occupancy) & ~friendlyPieces;
		while (pieceAttacks)
		{
			dst = getFirstBitSquare(pieceAttacks);
			insert(moveList, createMove(B + offset, src, dst, NO_SPECIAL, none));
			clear_lsb(pieceAttacks);
			//printf("%d. B%s\n", pos.fullMove, squareNames[dst]);
		}
		clear_lsb(pieceBB);
	}
}

void generateRookMoves(GameState pos, int turn, int offset, Node **moveList)
{
	int src, dst;
	
	U64 pieceBB, pieceAttacks, friendlyPieces, occupancy;
	occupancy = pos.occupancies[BOTH];
	friendlyPieces = pos.occupancies[turn];
	
	// Generate Rook Moves
	pieceBB = pos.pieceBitboards[R + offset];
	while (pieceBB)
	{
		src = getFirstBitSquare(pieceBB);
		pieceAttacks = getRookAttacks(src, occupancy) & ~friendlyPieces;
		while (pieceAttacks)
		{
			dst = getFirstBitSquare(pieceAttacks);
			insert(moveList, createMove(R + offset, src, dst, NO_SPECIAL, none));
			clear_lsb(pieceAttacks);
			//printf("%d. R%s\n", pos.fullMove, squareNames[dst]);
		}
		clear_lsb(pieceBB);
	}
}

void generateQueenMoves(GameState pos, int turn, int offset, Node **moveList)
{
	int src, dst;
	
	U64 pieceBB, pieceAttacks, friendlyPieces, occupancy;
	occupancy = pos.occupancies[BOTH];
	friendlyPieces = pos.occupancies[turn];
	
	// Generate Queen Moves
	pieceBB = pos.pieceBitboards[Q + offset];
	while (pieceBB)
	{
		src = getFirstBitSquare(pieceBB);
		pieceAttacks = getQueenAttacks(src, occupancy) & ~friendlyPieces;
		while (pieceAttacks)
		{
			dst = getFirstBitSquare(pieceAttacks);
			insert(moveList, createMove(Q + offset, src, dst, NO_SPECIAL, none));
			clear_lsb(pieceAttacks);
			//printf("%d. Q%s\n", pos.fullMove, squareNames[dst]);
		}
		clear_lsb(pieceBB);
	}
}

Node *generateMoves(GameState pos, int *size)
{
	Node *pseudoList = NULL;
	Node *moveList = NULL;
	Node *temp = NULL;
	GameState tempState;
	int turn = pos.turn;
	int offset = 6 * turn;
	int kingLocation;
	int moveCount = 0;
	
	generatePawnMoves(pos, turn, offset, &pseudoList);
	generateKingMoves(pos, turn, offset, &pseudoList);
	generateKnightMoves(pos, turn, offset, &pseudoList);
	generateBishopMoves(pos, turn, offset, &pseudoList);
	generateRookMoves(pos, turn, offset, &pseudoList);
	generateQueenMoves(pos, turn, offset, &pseudoList);
	
	// TODO only return legal moves
	temp = pseudoList;
	while (temp != NULL)
	{
		tempState = playMove(pos, temp->move);
		kingLocation = getFirstBitSquare(tempState.pieceBitboards[K + offset]);
		if (isSquareAttacked(tempState, kingLocation, (turn == WHITE) ? BLACK : WHITE) == 0)
		{
			moveCount++;
			insert(&moveList, temp->move);
		}
		//prev = temp;
		temp = temp->next;
		//free(prev);
	}
	*size = moveCount;
	deleteList(pseudoList);
	return moveList;
}

void printMoveList(Node *head, GameState pos)
{
	struct node *temp;
	temp = head;

	if (temp == NULL)
	{
		int offset = 6 * pos.turn;
		int kingLocation = getFirstBitSquare(pos.pieceBitboards[K + offset]);
		if (isSquareAttacked(pos, kingLocation, (pos.turn == WHITE) ? BLACK : WHITE) == 0)
		{
			printf("Stalemate\n");
		}
		else
		{
			printf("Checkmate\n");
		}
	}

	while (temp != NULL)
	{
		printf("%d. ", pos.fullMove);
		if (temp->move.special == NO_SPECIAL || temp->move.special == EN_PASSANT_SPECIAL)
		{
			printf("%s%s-%s\n", pieceNotation[temp->move.piece], squareNames[temp->move.src], squareNames[temp->move.dst]);
		}
		else if (temp->move.piece == K || temp->move.piece == k)
		{
			printf("%s\n", (temp->move.special == OO_SPECIAL) ? "O-O" : "O-O-O");
		}
		else
		{
			printf("%s%s-%s=%s\n", pieceNotation[temp->move.piece], squareNames[temp->move.src], squareNames[temp->move.dst], pieceNotation[temp->move.special]);
		}
		
		temp = temp->next;
	}
}
