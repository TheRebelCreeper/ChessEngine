#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "position.h"

U64 PIECE_BITBOARDS[12];

int flip = 0;

struct GameState state;

char *pieceChars[13] = {"P", "N", "B", "R", "Q", "K", "p", "n", "b", "r", "q", "k", " "};

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
	
	memset(PIECE_BITBOARDS, 0ULL, sizeof(PIECE_BITBOARDS));
	state.turn = 1;
	state.castlingRights = 0;
	state.enpessantSquare = -1;
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
				set_square(PIECE_BITBOARDS[piece], square);
			}
			index++;
		}
		token = strtok(NULL, DELIMS);
	}
	
	// First gamestate is side to move
	if (strlen(token) == 1 && (token[0] == 'w' || token[0] == 'W' ))
	{
		state.turn = 1;
	}
	else if (strlen(token) == 1 && (token[0] == 'b' || token[0] == 'B'))
	{
		state.turn = 0;
	}
	else
	{
		printf("Invalid FEN\n");
		exit(EXIT_FAILURE);	
	}
	
	// Get castling gamestate
	token = strtok(NULL, DELIMS);
	state.castlingRights = getCastlingRights(token);
	
	// Get enpessantSquare
	token = strtok(NULL, DELIMS);
	state.enpessantSquare = (token[0] == '-') ? -1 : getSquareFromNotation(token);
	
	// TODO get move counters
	token = strtok(NULL, DELIMS);
	state.halfMoveClock = atoi(token);
	token = strtok(NULL, DELIMS);
	state.fullMove = atoi(token);
}

void initStartingPosition()
{
	loadFEN(STARTING_FEN);
}

U64 getBlackPieces()
{
	return PIECE_BITBOARDS[p] | PIECE_BITBOARDS[n] | PIECE_BITBOARDS[b] | PIECE_BITBOARDS[r] | PIECE_BITBOARDS[q] | PIECE_BITBOARDS[k];
}

U64 getWhitePieces()
{
	return PIECE_BITBOARDS[P] | PIECE_BITBOARDS[N] | PIECE_BITBOARDS[B] | PIECE_BITBOARDS[R] | PIECE_BITBOARDS[Q] | PIECE_BITBOARDS[K];
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
		if (get_square(PIECE_BITBOARDS[i], square))
			return i;
	}
	return NO_PIECE;
}

char* getCharAtSquare(int square)
{
	return pieceChars[getPieceAtSquare(square)];
}

void printBoard()
{
	char* piece;
	int rank, file, square;
	printf("  +---+---+---+---+---+---+---+---+\n");
	if (state.turn)
	{
		for (rank = 7; rank >= 0; rank--)
		{
			printf("%d ", rank + 1);
			for (file = 0; file < 8; file++)
			{
				square = rank * 8 + file;
				piece = getCharAtSquare(square);
				printf("| %s ", piece);
			}
			printf("|\n");
			printf("  +---+---+---+---+---+---+---+---+\n");
		}
		printf("    a   b   c   d   e   f   g   h\n");
	}
	else
	{
		for (rank = 0; rank < 8; rank++)
		{
			printf("%d ", rank + 1);
			for (file = 7; file >= 0; file--)
			{
				square = rank * 8 + file;
				piece = getCharAtSquare(square);
				printf("| %s ", piece);
			}
			printf("|\n");
			printf("  +---+---+---+---+---+---+---+---+\n");
		}
		printf("    h   g   f   e   d   c   b   a\n");
	}
	
	printf("\n%s to move\n", (state.turn) ? "White" : "Black");
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
	printf("En Pessant Square: %d\n", state.enpessantSquare);
	printf("Halfmove Clock: %d\n", state.halfMoveClock);
	printf("Move: %d\n", state.fullMove);
}