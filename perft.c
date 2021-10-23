#include "perft.h"

U64 perft(int depth, GameState *pos)
{
	MoveList moveList;
	int size;
	U64 sum = 0;
	if (depth == 0)
	{
		return 1ULL;
	}
	moveList = generateMoves(pos, &size);
	if (depth == 1)
	{
		return (U64)size;
	}
	
	for (int i = 0; i < moveList.nextOpen; i++)
	{
		if (moveList.list[i].legal == 1)
		{
			GameState newState = playMove(pos, moveList.list[i]);
			sum += perft(depth - 1, &newState);
		}
	}
	return sum;
}

U64 perftDivide(int depth, GameState *pos)
{
	MoveList moveList;
	int size, i;
	U64 sum = 0;
	
	if (depth == 0)
	{
		return 1ULL;
	}
	
	moveList = generateMoves(pos, &size);
	printf("Perft results for depth %d:\n", depth);

	#pragma omp parallel for num_threads(12) shared(moveList) reduction(+:sum)
	for (i = 0; i < moveList.nextOpen; i++)
	{
		Move current = moveList.list[i];
		if (current.legal == 1)
		{
			GameState newState = playMove(pos, moveList.list[i]);
			U64 res = perft(depth - 1, &newState);
			sum += res;
			if (current.special == NO_SPECIAL || current.special == EN_PASSANT_SPECIAL || current.piece == K || current.piece == k)
			{
				printf("%s%s", squareNames[current.src], squareNames[current.dst]);
			}
			else
			{
				printf("%s%s=%s", squareNames[current.src], squareNames[current.dst], pieceNotation[current.special]);
			}
			printf(": %llu\n", res);
		}
	}
	
	return sum;
}

void runPerft(int depth, GameState *pos)
{
	U64 size;
	double start, finish;
	start = omp_get_wtime();
	size = perftDivide(depth, pos);
	printf("Nodes searched: %llu\n\n", size);
	finish = omp_get_wtime();
	printf("Finished perft in %f seconds\n", finish - start);
	printf("NPS: %f\n", size / (finish - start));
}