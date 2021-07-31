#include <stdio.h>
#include <stdlib.h>
#include "bitboard.h"

int main()
{
	U64 board = (Rank4 | Rank5) & (FileD | FileE);
	//set_square(board, e4);
	//clear_square(board, e4);
	print_bitboard(board);
	return 0;
}