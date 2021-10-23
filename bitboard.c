#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "bitboard.h"
#include "magic.h"

const U64 FileA = 0x0101010101010101ULL;
const U64 FileB = 0x0101010101010101ULL << 1;
const U64 FileC = 0x0101010101010101ULL << 2;
const U64 FileD = 0x0101010101010101ULL << 3;
const U64 FileE = 0x0101010101010101ULL << 4;
const U64 FileF = 0x0101010101010101ULL << 5;
const U64 FileG = 0x0101010101010101ULL << 6;
const U64 FileH = 0x0101010101010101ULL << 7;

const U64 Rank1 = 0xFFULL;
const U64 Rank2 = 0xFFULL << (8 * 1);
const U64 Rank3 = 0xFFULL << (8 * 2);
const U64 Rank4 = 0xFFULL << (8 * 3);
const U64 Rank5 = 0xFFULL << (8 * 4);
const U64 Rank6 = 0xFFULL << (8 * 5);
const U64 Rank7 = 0xFFULL << (8 * 6);
const U64 Rank8 = 0xFFULL << (8 * 7);

int RBits[64] = {
	12, 11, 11, 11, 11, 11, 11, 12,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	12, 11, 11, 11, 11, 11, 11, 12
};

int BBits[64] = {
	6, 5, 5, 5, 5, 5, 5, 6,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 5, 5, 5, 5, 5, 5, 6
};

char *squareNames[65] = {
	"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
	"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
	"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
	"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
	"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
	"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
	"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
	"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8", "-"
};

// Algorithm from Brian Kernighan
int countBits(U64 board)
{
	return __builtin_popcountll(board);
	/*int count = 0;
	while (board)
	{
		count++;
		board &= board - 1;
	}
	return count;
	*/
}

int getFirstBitSquare(U64 board)
{
	return __builtin_ffsll(board) - 1;
}

U64 occupancyFromIndex(int index, U64 board)
{
	int i, square;
	U64 occupancy = 0ULL;
	int bits = countBits(board);
   
	// Cannot be easily parallelized since order matters regarding 1st least significant bit
	for (i = 0; i < bits; i++)
	{
		square = getFirstBitSquare(board);
		clear_square(board, square);
      
		// Check if the ith bit is marked in the index
		if (index & (1 << i))
		{
			// Add the square to occupancy
			set_square(occupancy, square);
		}
	}
	return occupancy;
}

U64 calculateKnightAttacks(int square)
{
	U64 pieceLocation = 0ULL;
	U64 attacks = 0ULL;
	set_square(pieceLocation, square);
	
	attacks |= (pieceLocation << 17 & ~FileA);              // Up 2 right 1
	attacks |= (pieceLocation << 15 & ~FileH);              // Up 2 left 1
	attacks |= (pieceLocation << 10 & ~(FileA | FileB));    // Up 1 right 2
	attacks |= (pieceLocation << 6 & ~(FileH | FileG));     // Up 1 left 2
	
	attacks |= (pieceLocation >> 17 & ~FileH);              // Down 2 left 1
	attacks |= (pieceLocation >> 15 & ~FileA);              // Down 2 right 1
	attacks |= (pieceLocation >> 10 & ~(FileH | FileG));    // Down 1 left 2
	attacks |= (pieceLocation >> 6 & ~(FileA | FileB));     // Down 1 right 2
	return attacks;
}

U64 calculateKingAttacks(int square)
{
	U64 pieceLocation = 0ULL;
	U64 attacks = 0ULL;
	set_square(pieceLocation, square);
		
	attacks |= (pieceLocation << 1 & ~FileA);    // Left
	attacks |= (pieceLocation << 7 & ~FileH);    // Top right
	attacks |= (pieceLocation << 8);             // Top 
	attacks |= (pieceLocation << 9 & ~FileA);    // Top left
	
	attacks |= (pieceLocation >> 1 & ~FileH);    // Right
	attacks |= (pieceLocation >> 7 & ~FileA);    // Bottom left
	attacks |= (pieceLocation >> 8);             // Bottom
	attacks |= (pieceLocation >> 9 & ~FileH);    // Bottom right

	return attacks;
}

U64 calculatePawnAttacks(int side, int square)
{
	U64 pieceLocation = 0ULL;
	U64 attacks = 0ULL;
	set_square(pieceLocation, square);
	
	// White attacks
	if (side == WHITE)
	{
		attacks |= (pieceLocation << 9 & ~FileA);    // Top left
		attacks |= (pieceLocation << 7 & ~FileH);    // Top right
	}
	// Black attacks
	else
	{
		attacks |= (pieceLocation >> 9 & ~FileH);    // Bottom right
		attacks |= (pieceLocation >> 7 & ~FileA);    // Bottom left
	}
	return attacks;
}

U64 calculateBishopOccupancy(int square)
{
	U64 occupancy = 0ULL;
	int rank, file;
	int r, f, s;
	
	rank = square / 8;
	file = square % 8;
	
	// Calculate bottom left
	r = rank - 1;
	f = file - 1;
	while(r > 0 && f > 0)
	{
		s = r * 8 + f;
		set_square(occupancy, s);
		r--;
		f--;
	}
	
	// Calculate top left
	r = rank + 1;
	f = file - 1;
	while(r < 7 && f > 0)
	{
		s = r * 8 + f;
		set_square(occupancy, s);
		r++;
		f--;
	}
	
	// Calculate top right
	r = rank + 1;
	f = file + 1;
	while(r < 7 && f < 7)
	{
		s = r * 8 + f;
		set_square(occupancy, s);
		r++;
		f++;
	}
	
	// Calculate top right
	r = rank - 1;
	f = file + 1;
	while(r > 0 && f < 7)
	{
		s = r * 8 + f;
		set_square(occupancy, s);
		r--;
		f++;
	}
	return occupancy;
}

U64 calculateRookOccupancy(int square)
{
	U64 occupancy = 0ULL;
	int rank, file;
	int r, f, s;
	
	rank = square / 8;
	file = square % 8;
	
	// Calculate left
	r = rank;
	f = file - 1;
	while(f > 0)
	{
		s = r * 8 + f;
		set_square(occupancy, s);
		f--;
	}
	
	// Calculate right
	r = rank;
	f = file + 1;
	while(f < 7)
	{
		s = r * 8 + f;
		set_square(occupancy, s);
		f++;
	}
	
	// Calculate top
	r = rank + 1;
	f = file;
	while(r < 7)
	{
		s = r * 8 + f;
		set_square(occupancy, s);
		r++;
	}
	
	// Calculate bottom
	r = rank - 1;
	f = file;
	while(r > 0)
	{
		s = r * 8 + f;
		set_square(occupancy, s);
		r--;
	}
	return occupancy;
}

U64 generateBishopAttacks(int square, U64 blockers)
{
	U64 occupancy = 0ULL;
	int rank, file;
	int r, f, s;
	
	rank = square / 8;
	file = square % 8;
	
	// Calculate bottom left
	r = rank - 1;
	f = file - 1;
	while(r >= 0 && f >= 0)
	{
		s = r * 8 + f;
		set_square(occupancy, s);
		if (get_square(blockers, s))
		{
			break;
		}
		r--;
		f--;
	}
	
	// Calculate top left
	r = rank + 1;
	f = file - 1;
	while(r <= 7 && f >= 0)
	{
		s = r * 8 + f;
		set_square(occupancy, s);
		if (get_square(blockers, s))
		{
			break;
		}
		r++;
		f--;
	}
	
	// Calculate top right
	r = rank + 1;
	f = file + 1;
	while(r <= 7 && f <= 7)
	{
		s = r * 8 + f;
		set_square(occupancy, s);
		if (get_square(blockers, s))
		{
			break;
		}
		r++;
		f++;
	}
	
	// Calculate top right
	r = rank - 1;
	f = file + 1;
	while(r >= 0 && f <= 7)
	{
		s = r * 8 + f;
		set_square(occupancy, s);
		if (get_square(blockers, s))
		{
			break;
		}
		r--;
		f++;
	}
	return occupancy;
}

U64 generateRookAttacks(int square, U64 blockers)
{
	U64 occupancy = 0ULL;
	int rank, file;
	int r, f, s;
	
	rank = square / 8;
	file = square % 8;
	
	// Calculate left
	r = rank;
	f = file - 1;
	while(f >= 0)
	{
		s = r * 8 + f;
		set_square(occupancy, s);
		if (get_square(blockers, s))
		{
			break;
		}
		f--;
	}
	
	// Calculate right
	r = rank;
	f = file + 1;
	while(f <= 7)
	{
		s = r * 8 + f;
		set_square(occupancy, s);
		if (get_square(blockers, s))
		{
			break;
		}
		f++;
	}
	
	// Calculate top
	r = rank + 1;
	f = file;
	while(r <= 7)
	{
		s = r * 8 + f;
		set_square(occupancy, s);
		if (get_square(blockers, s))
		{
			break;
		}
		r++;
	}
	
	// Calculate bottom
	r = rank - 1;
	f = file;
	while(r >= 0)
	{
		s = r * 8 + f;
		set_square(occupancy, s);
		if (get_square(blockers, s))
		{
			break;
		}
		r--;
	}
	return occupancy;
}

inline U64 getBishopAttacks(int square, U64 blockers)
{
	int mIndex;
	blockers &= bishopOccupancy[square];
	blockers *= bishopMagic[square];
	mIndex = blockers >> (64 - BBits[square]);
	
	return bishopAttacks[square][mIndex];
}

inline U64 getRookAttacks(int square, U64 blockers)
{
	int mIndex;
	blockers &= rookOccupancy[square];
	blockers *= rookMagic[square];
	mIndex = blockers >> (64 - RBits[square]);
	
	return rookAttacks[square][mIndex];
}

inline U64 getQueenAttacks(int square, U64 blockers)
{
	return getBishopAttacks(square, blockers) | getRookAttacks(square, blockers);
}

void initSliders()
{
	int square;
	
	#ifdef DEBUG
	double start, finish;
	start = omp_get_wtime();
	#endif
	
	for (square = 0; square < 64; square++)
	{
		bishopOccupancy[square] = calculateBishopOccupancy(square);
		rookOccupancy[square] = calculateRookOccupancy(square);
		
		bishopMagic[square] = find_magic_number(square, BBits[square], 1);
		rookMagic[square] = find_magic_number(square, RBits[square], 0);
	}
	
	//#pragma omp parallel for private(square) shared(BishopAttacks, RookAttacks)
	for (square = 0; square < 64; square++)
	{
		for(int index = 0; index < (1 << BBits[square]); index++)
		{
			U64 occupancy = occupancyFromIndex(index, bishopOccupancy[square]);
			int mIndex = (occupancy * bishopMagic[square]) >> (64 - BBits[square]);
			bishopAttacks[square][mIndex] = generateBishopAttacks(square, occupancy);
		}
		
		for(int index = 0; index < (1 << RBits[square]); index++)
		{
			U64 occupancy = occupancyFromIndex(index, rookOccupancy[square]);
			int mIndex = (occupancy * rookMagic[square]) >> (64 - RBits[square]);
			rookAttacks[square][mIndex] = generateRookAttacks(square, occupancy);
		}
	}
	
	#ifdef DEBUG
	finish = omp_get_wtime();
	printf("Generated sliders in %f\n", finish - start);
	#endif
}

void initLeapers()
{
	int square;
	
	#ifdef DEBUG
	double start, finish;
	start = omp_get_wtime();
	#endif
	
	//#pragma omp parallel for private(square) shared(KnightAttacks, KingAttacks, PawnAttacks)
	for (square = 0; square < 64; square++)
	{
		knightAttacks[square] = calculateKnightAttacks(square);
		kingAttacks[square] = calculateKingAttacks(square);
		pawnAttacks[0][square] = calculatePawnAttacks(0, square);
		pawnAttacks[1][square] = calculatePawnAttacks(1, square);
	}
	
	#ifdef DEBUG
	finish = omp_get_wtime();
	printf("Generated leapers in %f\n", finish - start);
	#endif
}

void initAttacks()
{
	initSliders();
	initLeapers();
}

void printBitboard(U64 board)
{
	int i, j;
	printf("  +---+---+---+---+---+---+---+---+\n");
	for (i = 7; i >= 0; i--)
	{
		printf("%d ", i + 1);
		for (j = 0; j < 8; j++)
		{
			printf("| %d ", get_square(board, i * 8 + j) ? 1 : 0);
		}
		printf("|\n");
		printf("  +---+---+---+---+---+---+---+---+\n");
	}
	printf("    a   b   c   d   e   f   g   h\n");
	printf("Bitboard: 0x%llx\n\n", board);
}
