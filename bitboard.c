#include <stdio.h>
#include "bitboard.h"

const U64 AFile = 0x0101010101010101ULL;
const U64 BFile = AFile << 1;
const U64 CFile = AFile << 2;
const U64 DFile = AFile << 3;
const U64 EFile = AFile << 4;
const U64 FFile = AFile << 5;
const U64 GFile = AFile << 6;
const U64 HFile = AFile << 7;

void print_bitboard(U64 board)
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