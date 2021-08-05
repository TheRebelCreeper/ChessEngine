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

U64 calculateKnightAttacks(int square)
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
	return attacks;
}

U64 calculateKingAttacks(int square)
{
	U64 pieceLocation = 0ULL;
	U64 attacks = 0ULL;
	set_square(pieceLocation, square);
		
	attacks |= (pieceLocation << 1 & ~FileA);
	attacks |= (pieceLocation << 7 & ~FileH);
	attacks |= (pieceLocation << 8);
	attacks |= (pieceLocation << 9 & ~FileA);
	
	attacks |= (pieceLocation >> 1 & ~FileH);
	attacks |= (pieceLocation >> 7 & ~FileA);
	attacks |= (pieceLocation >> 8);
	attacks |= (pieceLocation >> 9 & ~FileH);

	return attacks;
}

U64 calculatePawnAttacks(int side, int square)
{
	U64 pieceLocation = 0ULL;
	U64 attacks = 0ULL;
	set_square(pieceLocation, square);
	
	if (side == 0)
	{
		attacks |= (pieceLocation << 9 & ~FileA);
		attacks |= (pieceLocation << 7 & ~FileH);
	}
	else
	{
		attacks |= (pieceLocation >> 9 & ~FileH);
		attacks |= (pieceLocation >> 7 & ~FileA);
	}
	return attacks;
}

U64 calculateBishopOccupancy(int pieceRank, int pieceFile)
{
	U64 occupancy = 0ULL;
	int r, f, s;
	
	// Calculate bottom left
	r = pieceRank - 1;
	f = pieceFile - 1;
	while(r > 0 && f > 0)
	{
		s = r * 8 + f;
		set_square(occupancy, s);
		r--;
		f--;
	}
	
	// Calculate top left
	r = pieceRank + 1;
	f = pieceFile - 1;
	while(r < 7 && f > 0)
	{
		s = r * 8 + f;
		set_square(occupancy, s);
		r++;
		f--;
	}
	
	// Calculate top right
	r = pieceRank + 1;
	f = pieceFile + 1;
	while(r < 7 && f < 7)
	{
		s = r * 8 + f;
		set_square(occupancy, s);
		r++;
		f++;
	}
	
	// Calculate top right
	r = pieceRank - 1;
	f = pieceFile + 1;
	while(r > 0 && f < 7)
	{
		s = r * 8 + f;
		set_square(occupancy, s);
		r--;
		f++;
	}
	return occupancy;
}

U64 calculateRookOccupancy(int pieceRank, int pieceFile)
{
	U64 occupancy = 0ULL;
	int r, f, s;
	
	// Calculate left
	r = pieceRank;
	f = pieceFile - 1;
	while(f > 0)
	{
		s = r * 8 + f;
		set_square(occupancy, s);
		f--;
	}
	
	// Calculate right
	r = pieceRank;
	f = pieceFile + 1;
	while(f < 7)
	{
		s = r * 8 + f;
		set_square(occupancy, s);
		f++;
	}
	
	// Calculate top
	r = pieceRank + 1;
	f = pieceFile;
	while(r < 7)
	{
		s = r * 8 + f;
		set_square(occupancy, s);
		r++;
	}
	
	// Calculate bottom
	r = pieceRank - 1;
	f = pieceFile;
	while(r > 0)
	{
		s = r * 8 + f;
		set_square(occupancy, s);
		r--;
	}
	return occupancy;
}

void initSliders()
{
	int square, rank, file;
	for (rank = 0; rank < 8; rank++)
	{
		for (file = 0; file < 8; file++)
		{
			square = rank * 8 + file;
			BishopOccupancy[square] = calculateBishopOccupancy(rank, file);
			RookOccupancy[square] = calculateRookOccupancy(rank, file);
		}
	}
}

void initLeapers()
{
	int square;
	
	#pragma omp parallel for
	for (square = 0; square < 64; square++)
	{
		KnightAttacks[square] = calculateKnightAttacks(square);
		KingAttacks[square] = calculateKingAttacks(square);
		PawnAttacks[0][square] = calculatePawnAttacks(0, square);
		PawnAttacks[1][square] = calculatePawnAttacks(1, square);
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
