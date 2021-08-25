#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "position.h"

int flip = 0;

struct GameState state;

#ifdef UNICODE_PIECES
char *pieceChars[13] = {"♙", "♘", "♗", "♖", "♕", "♔", "♟", "♞", "♝", "♜", "♛", "♚", " "};
#else
char *pieceChars[13] = {"P", "N", "B", "R", "Q", "K", "p", "n", "b", "r", "q", "k", " "};
#endif

int getSquareFromNotation(char *str)
{
	if (strlen(str) != 2)
	{
		printf("Invalid FEN\n");
		exit(EXIT_FAILURE);
	}
	return (str[1] - '0' - 1) * 8 + (tolower(str[0]) - 'a');
}

int getPieceFromChar(char c)
{
	if (c == 'P')
		return P;
	if (c == 'N')
		return N;
	if (c == 'B')
		return B;
	if (c == 'R')
		return R;
	if (c == 'Q')
		return Q;
	if (c == 'K')
		return K;
	if (c == 'p')
		return p;
	if (c == 'n')
		return n;
	if (c == 'b')
		return b;
	if (c == 'r')
		return r;
	if (c == 'q')
		return q;
	if (c == 'k')
		return k;
	else
		return NO_PIECE;
}

char getCastlingRights(char *str)
{
	char rights = 0;
	if (strlen(str) > 4)
	{
		printf("Invalid FEN\n");
		exit(EXIT_FAILURE);
	}
	if (strchr(str, 'K') != NULL)
	{
		rights |= WHITE_OO;
	}
	if (strchr(str, 'Q') != NULL)
	{
		rights |= WHITE_OOO;
	}
	if (strchr(str, 'k') != NULL)
	{
		rights |= BLACK_OO;
	}
	if (strchr(str, 'q') != NULL)
	{
		rights |= BLACK_OOO;
	}
	
	return rights;
}

U64 getBlackPieces()
{
	return state.pieceBitboards[p] | state.pieceBitboards[n] | state.pieceBitboards[b] | state.pieceBitboards[r] | state.pieceBitboards[q] | state.pieceBitboards[k];
}

U64 getWhitePieces()
{
	return state.pieceBitboards[P] | state.pieceBitboards[N] | state.pieceBitboards[B] | state.pieceBitboards[R] | state.pieceBitboards[Q] | state.pieceBitboards[K];
}

U64 getAllPieces()
{
	return getBlackPieces() | getWhitePieces();
}

int getPieceAtSquare(int square)
{
	int i;
	for (i = 0; i < 12; i++)
	{
		if (get_square(state.pieceBitboards[i], square))
			return i;
	}
	return NO_PIECE;
}

char isSquareAttacked(int square, int byColor)
{
	/*	Will use individual bits to indicate which pieces are attacking the square
		This is only really used for debugging purposes
		Bit 0 - Pawn
		Bit 1 - Knight
		Bit 2 - Bishop
		Bit 3 - Rook
		Bit 4 - Queen
		Bit 5 - King
	*/
	char attackers = 0;
	
	
	int colorOffset = (byColor == WHITE) ? 0 : 6;
	int pawnAttackColor = (byColor == WHITE) ? 1 : 0;
	U64 occupancy = getAllPieces();
	
	if (kingAttacks[square] & state.pieceBitboards[K + colorOffset])
		attackers |= (1 << 5);
	if (knightAttacks[square] & state.pieceBitboards[N + colorOffset])
		attackers |= (1 << 1);
	if (pawnAttacks[pawnAttackColor][square] & state.pieceBitboards[P + colorOffset])
		attackers |= 1;
	if (getBishopAttack(square, occupancy) & state.pieceBitboards[B + colorOffset])
		attackers |= (1 << 2);
	if (getRookAttack(square, occupancy) & state.pieceBitboards[R + colorOffset])
		attackers |= (1 << 3);
	if (getQueenAttack(square, occupancy) & state.pieceBitboards[Q + colorOffset])
		attackers |= (1 << 4);
	return attackers;
		
}

void loadFEN(char *fen)
{
	
	int rank, file, square, piece, index, length;
	char *str = NULL;
	char *token = NULL;
	
	length = strlen(fen);
	str = malloc(length + 1);
	if (str == NULL)
	{
		perror("Malloc error in loadFEN");
		exit(EXIT_FAILURE);
	}
	strncpy(str, fen, length);
	
	memset(state.pieceBitboards, 0ULL, sizeof(state.pieceBitboards));
	state.turn = 0;
	state.castlingRights = 0;
	state.enpassantSquare = -1;
	state.halfMoveClock = 0;
	state.fullMove = 1;
	
	token = strtok(str, DELIMS);
	// For loops read in pieces
	for (rank = 7; rank >= 0; rank--)
	{
		index = 0;
		for (file = 0; file < 8; file++)
		{
			piece = getPieceFromChar(token[index]);
			if (isdigit(token[index]))
			{
				file += (token[index] - '0') - 1;
			}
			else
			{
				if (piece == NO_PIECE)
				{
					printf("Invalid FEN\n");
					exit(EXIT_FAILURE);
				}
				square = rank * 8 + file;
				set_square(state.pieceBitboards[piece], square);
			}
			index++;
		}
		token = strtok(NULL, DELIMS);
	}
	
	// First gamestate is side to move
	if (strlen(token) == 1 && (token[0] == 'w' || token[0] == 'W' ))
	{
		state.turn = WHITE;
	}
	else if (strlen(token) == 1 && (token[0] == 'b' || token[0] == 'B'))
	{
		state.turn = BLACK;
	}
	else
	{
		printf("Invalid FEN\n");
		exit(EXIT_FAILURE);	
	}
	
	// Get castling gamestate
	token = strtok(NULL, DELIMS);
	state.castlingRights = getCastlingRights(token);
	
	// Get enpassantSquare
	token = strtok(NULL, DELIMS);
	state.enpassantSquare = (token[0] == '-') ? -1 : getSquareFromNotation(token);
	
	// TODO get move counters
	token = strtok(NULL, DELIMS);
	state.halfMoveClock = atoi(token);
	token = strtok(NULL, DELIMS);
	state.fullMove = atoi(token);
	
	free(str);
}

void initStartingPosition()
{
	loadFEN(STARTING_FEN);
}


void printBoard()
{
	char* piece;
	int rank, file, square;
	printf("  +---+---+---+---+---+---+---+---+\n");
	for (rank = 0; rank < 8; rank++)
	{
		if (state.turn == WHITE)
			printf("%d ", 8 - rank);
		else
			printf("%d ", rank + 1);
			
		for (file = 0; file < 8; file++)
		{
			square = (state.turn == WHITE) ? ((7 - rank) * 8 + file) : (rank * 8 + (7 - file));
			piece = pieceChars[getPieceAtSquare(square)];
			
			#ifndef _WIN32
			if ((!(square & 1) && !(rank & 1)) || ((square & 1) && rank & 1))
				printf("|\033[38;2;0;0;0;48;2;245;245;220m");
			else
				printf("|\033[38;2;0;0;0;48;2;152;118;84m");
			#endif
			printf(" %s ", piece);
			
			#ifndef _WIN32
			printf("\033[39;49m");
			#endif
		}
		printf("|\n");
		printf("  +---+---+---+---+---+---+---+---+\n");
	}
	if (state.turn == WHITE)
		printf("    a   b   c   d   e   f   g   h\n");
	else
		printf("    h   g   f   e   d   c   b   a\n");
	
	printf("\n%s to move\n", (state.turn == WHITE) ? "White" : "Black");
	printf("Castling Rights: ");
	if (state.castlingRights & WHITE_OO)
		printf("K");
	if (state.castlingRights & WHITE_OOO)
		printf("Q");
	if (state.castlingRights & BLACK_OO)
		printf("k");
	if (state.castlingRights & BLACK_OOO)
		printf("q");
	if (!state.castlingRights)
		printf("-");
	printf("\n");
	printf("En Passant Square: %d\n", state.enpassantSquare);
	printf("Halfmove Clock: %d\n", state.halfMoveClock);
	printf("Move: %d\n", state.fullMove);
}
