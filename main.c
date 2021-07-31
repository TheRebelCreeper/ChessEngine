#include <stdio.h>
#include <stdlib.h>
#include "bitboard.h"

int main()
{
	//U64 board = 0x0101010101010101ULL << 2;
	//set_square(board, e4);
	//clear_square(board, e4);
	print_bitboard(AFile);
	print_bitboard(BFile);
	print_bitboard(CFile);
	print_bitboard(DFile);
	print_bitboard(EFile);
	print_bitboard(FFile);
	print_bitboard(GFile);
	print_bitboard(HFile);
	return 0;
}