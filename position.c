#include <stdio.h>
#include "position.h"

U64 WHITE_PAWNS = 0ULL;
U64 WHITE_KNIGHTS = 0ULL;
U64 WHITE_BISHOPS = 0ULL;
U64 WHITE_ROOKS = 0ULL;
U64 WHITE_QUEENS = 0ULL;
U64 WHITE_KINGS = 0ULL;

U64 BLACK_PAWNS = 0ULL;
U64 BLACK_KNIGHTS = 0ULL;
U64 BLACK_BISHOPS = 0ULL;
U64 BLACK_ROOKS = 0ULL;
U64 BLACK_QUEENS = 0ULL;
U64 BLACK_KINGS = 0ULL;

int flip = 0;

char *pieceChars[13] = {"P", "N", "B", "R", "Q", "K", "p", "n", "b", "r", "q", "k", " "};

void initStartingPosition()
{
	WHITE_PAWNS = Rank2;
	set_square(WHITE_KNIGHTS, b1);
	set_square(WHITE_KNIGHTS, g1);
	set_square(WHITE_BISHOPS, c1);
	set_square(WHITE_BISHOPS, f1);
	set_square(WHITE_ROOKS, a1);
	set_square(WHITE_ROOKS, h1);
	set_square(WHITE_QUEENS, d1);
	set_square(WHITE_KINGS, e1);
	
	BLACK_PAWNS = Rank7;
	set_square(BLACK_KNIGHTS, b8);
	set_square(BLACK_KNIGHTS, g8);
	set_square(BLACK_BISHOPS, c8);
	set_square(BLACK_BISHOPS, f8);
	set_square(BLACK_ROOKS, a8);
	set_square(BLACK_ROOKS, h8);
	set_square(BLACK_QUEENS, d8);
	set_square(BLACK_KINGS, e8);
	
	set_square(WHITE_PAWNS, e4);
	set_square(BLACK_PAWNS, c5);
	clear_square(WHITE_PAWNS, e2);
	clear_square(BLACK_PAWNS, c7);
}

U64 getBlackPieces()
{
	return BLACK_PAWNS | BLACK_KNIGHTS | BLACK_BISHOPS | BLACK_ROOKS | BLACK_QUEENS | BLACK_KINGS;
}

U64 getWhitePieces()
{
	return WHITE_PAWNS | WHITE_KNIGHTS | WHITE_BISHOPS | WHITE_ROOKS | WHITE_QUEENS | WHITE_KINGS;
}

U64 getAllPieces()
{
	return getBlackPieces() | getWhitePieces();
}

int getPieceAtSquare(int square)
{
	if (get_square(WHITE_PAWNS, square))
		return P;
	if (get_square(WHITE_KNIGHTS, square))
		return N;
	if (get_square(WHITE_BISHOPS, square))
		return B;
	if (get_square(WHITE_ROOKS, square))
		return R;
	if (get_square(WHITE_QUEENS, square))
		return Q;
	if (get_square(WHITE_KINGS, square))
		return K;
	if (get_square(BLACK_PAWNS, square))
		return p;
	if (get_square(BLACK_KNIGHTS, square))
		return n;
	if (get_square(BLACK_BISHOPS, square))
		return b;
	if (get_square(BLACK_ROOKS, square))
		return r;
	if (get_square(BLACK_QUEENS, square))
		return q;
	if (get_square(BLACK_KINGS, square))
		return k;
	else
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
	if (!flip)
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
}
