#include <stdio.h>
#include <omp.h>
#include "bitboard.h"

const U64 FileA = 0x0101010101010101ULL;
const U64 FileB = FileA << 1;
const U64 FileC = FileA << 2;
const U64 FileD = FileA << 3;
const U64 FileE = FileA << 4;
const U64 FileF = FileA << 5;
const U64 FileG = FileA << 6;
const U64 FileH = FileA << 7;

const U64 Rank1 = 0xFFULL;
const U64 Rank2 = Rank1 << (8 * 1);
const U64 Rank3 = Rank1 << (8 * 2);
const U64 Rank4 = Rank1 << (8 * 3);
const U64 Rank5 = Rank1 << (8 * 4);
const U64 Rank6 = Rank1 << (8 * 5);
const U64 Rank7 = Rank1 << (8 * 6);
const U64 Rank8 = Rank1 << (8 * 7);

void calculateKnightAttacks()
{
	int square;
	#pragma omp parallel for
	for (square = 0; square < 64; square++)
	{
		U64 pieceLocation = 0ULL;
		U64 attacks = 0ULL;
		set_square(pieceLocation, square);
		
		attacks |= (pieceLocation << 17 & ~FileA);
		attacks |= (pieceLocation << 15 & ~FileH);
		attacks |= (pieceLocation << 10 & ~(FileA | FileB));
		attacks |= (pieceLocation << 6 & ~(FileH | FileG));
		
		attacks |= (pieceLocation >> 17 & ~FileH);
		attacks |= (pieceLocation >> 15 & ~FileA);
		attacks |= (pieceLocation >> 10 & ~(FileH | FileG));
		attacks |= (pieceLocation >> 6 & ~(FileA | FileB));
		KnightMoves[square] = attacks;
	}
}

void printBitboard(U64 board)
{
	int i, j;
	for (i = 7; i >= 0; i--)
	{
		printf("%d\t", i + 1);
		for (j = 0; j < 8; j++)
		{
			printf(" %d", get_square(board, i * 8 + j) ? 1 : 0);
		}
		printf("\n");
	}
	printf("\n \t a b c d e f g h\n");
	printf("Bitboard: 0x%llx\n", board);
}
