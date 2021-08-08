#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

/**********************************\
 ==================================
 
           Random numbers
 
 ==================================
\**********************************/

// pseudo random number state
unsigned int random_state = 1804289383;

// generate 32-bit pseudo legal numbers
unsigned int get_random_U32_number()
{
    // get current state
    unsigned int number = random_state;
    
    // XOR shift algorithm
    number ^= number << 13;
    number ^= number >> 17;
    number ^= number << 5;
    
    // update random number state
    random_state = number;
    
    // return random number
    return number;
}

// generate 64-bit pseudo legal numbers
U64 get_random_U64_number()
{
    // define 4 random numbers
    U64 n1, n2, n3, n4;
    
    // init random numbers slicing 16 bits from MS1B side
    n1 = (U64)(get_random_U32_number()) & 0xFFFF;
    n2 = (U64)(get_random_U32_number()) & 0xFFFF;
    n3 = (U64)(get_random_U32_number()) & 0xFFFF;
    n4 = (U64)(get_random_U32_number()) & 0xFFFF;
    
    // return random number
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

// generate magic number candidate
U64 generate_magic_number()
{
    return get_random_U64_number() & get_random_U64_number() & get_random_U64_number();
}

// Algorithm from Brian Kernighan
int countBits(U64 board)
{
   int count = 0;
   while (board)
   {
      count++;
      board &= board - 1;
   }
   return count;
}

int getFirstBitSquare(U64 board)
{
   if (board)
   {
      return countBits((board & -board) - 1);
   }
   else
   {
      // No bits set, doesn't make sense to return a square
      return -1;
   }
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
      
      // Check if the ith is marked in the index
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
	if (side == 0)
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

U64 getBishopAttack(int square, U64 blockers)
{
	int mIndex;
	blockers &= BishopOccupancy[square];
	blockers *= BishopMagic[square];
	mIndex = blockers >> (64 - BBits[square]);
	
	return BishopAttacks[square][mIndex];
}

/**********************************\
 ==================================
 
               Magics
 
 ==================================
\**********************************/

// find appropriate magic number
U64 find_magic_number(int square, int relevant_bits, int bishop)
{
    // init occupancies
    U64 occupancies[4096];
    
    // init attack tables
    U64 attacks[4096];
    
    // init used attacks
    U64 used_attacks[4096];
    
    // init attack mask for a current piece
    U64 attack_mask = bishop ? calculateBishopOccupancy(square) : calculateRookOccupancy(square);
    
    // init occupancy indicies
    int occupancy_indicies = 1 << relevant_bits;
    
    // loop over occupancy indicies
    for (int index = 0; index < occupancy_indicies; index++)
    {
        // init occupancies
        occupancies[index] = occupancyFromIndex(index, attack_mask);
        
        // init attacks
        attacks[index] = bishop ? generateBishopAttacks(square, occupancies[index]) :
                                    generateRookAttacks(square, occupancies[index]);
    }
    
    // test magic numbers loop
    for (int random_count = 0; random_count < 100000000; random_count++)
    {
        // generate magic number candidate
        U64 magic_number = generate_magic_number();
        
        // skip inappropriate magic numbers
        if (countBits((attack_mask * magic_number) & 0xFF00000000000000) < 6) continue;
        
        // init used attacks
        memset(used_attacks, 0ULL, sizeof(used_attacks));
        
        // init index & fail flag
        int index, fail;
        
        // test magic index loop
        for (index = 0, fail = 0; !fail && index < occupancy_indicies; index++)
        {
            // init magic index
            int magic_index = (int)((occupancies[index] * magic_number) >> (64 - relevant_bits));
            
            // if magic index works
            if (used_attacks[magic_index] == 0ULL)
                // init used attacks
                used_attacks[magic_index] = attacks[index];
            
            // otherwise
            else if (used_attacks[magic_index] != attacks[index])
                // magic index doesn't work
                fail = 1;
        }
        
        // if magic number works
        if (!fail)
            // return it
            return magic_number;
    }
    
    // if magic number doesn't work
    printf("  Magic number fails!\n");
    return 0ULL;
}

// init magic numbers
void init_magic_numbers()
{
    // loop over 64 board squares
    for (int square = 0; square < 64; square++)
        // init rook magic numbers
        RookMagic[square] = find_magic_number(square, RBits[square], 0);

    // loop over 64 board squares
    for (int square = 0; square < 64; square++)
        // init bishop magic numbers
        BishopMagic[square] = find_magic_number(square, BBits[square], 1);
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
		BishopOccupancy[square] = calculateBishopOccupancy(square);
		RookOccupancy[square] = calculateRookOccupancy(square);
		//BishopMagic[square] = find_magic(square, BBits[square], 1);
		//RookMagic[square] = find_magic(square, RBits[square], 0);
		
		
	}
	
	init_magic_numbers();
	
	for (square = 0; square < 64; square++)
	{
		for(int index = 0; index < (1 << BBits[square]); index++)
		{
			U64 occupancy = occupancyFromIndex(index, BishopOccupancy[square]);
			int mIndex = (occupancy * BishopMagic[square]) >> (64 - countBits(BishopOccupancy[square]));
			BishopAttacks[square][mIndex] = generateBishopAttacks(square, occupancy);
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
	
	#pragma omp parallel for private(square) shared(KnightAttacks, KingAttacks, PawnAttacks)
	for (square = 0; square < 64; square++)
	{
		KnightAttacks[square] = calculateKnightAttacks(square);
		KingAttacks[square] = calculateKingAttacks(square);
		PawnAttacks[0][square] = calculatePawnAttacks(0, square);
		PawnAttacks[1][square] = calculatePawnAttacks(1, square);
	}
	
	#ifdef DEBUG
	finish = omp_get_wtime();
	printf("Generated leapers in %f\n", finish - start);
	#endif
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
