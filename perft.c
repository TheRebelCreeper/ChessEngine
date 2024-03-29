#include "perft.h"
#include "util.h"

extern int NUM_THREADS;

U64 perft(int depth, GameState *pos)
{
	MoveList moveList;
	int size, legal;
	U64 sum = 0;
	if (depth == 0)
	{
		return 1ULL;
	}
	
	moveList = generateMoves(pos, &size);
	
	for (int i = 0; i < size; i++)
	{
		GameState newState = playMove(pos, moveList.list[i], &legal);
		if (legal)
		{
			sum += perft(depth - 1, &newState);
		}
	}
	return sum;
}

U64 perftDivide(int depth, GameState *pos)
{
	MoveList moveList;
	int size, i, legal;
	U64 sum = 0;
	
	if (depth == 0)
	{
		return 1ULL;
	}
	
	moveList = generateMoves(pos, &size);
	printf("Perft results for depth %d:\n", depth);

	for (i = 0; i < size; i++)
	{
		Move current = moveList.list[i];
		GameState newState = playMove(pos, current, &legal);
		if (legal == 1)
		{
			U64 res = perft(depth - 1, &newState);
			sum += res;
			printMove(current);
			printf(": %llu\n", res);
		}
	}
	return sum;
}

U64 runPerft(int depth, GameState *pos)
{
	U64 size;
	unsigned int start, finish;
	double seconds;
	start = GetTimeMs();
	size = perftDivide(depth, pos);
	printf("Nodes searched: %llu\n\n", size);
	finish = GetTimeMs();
	seconds = (finish - start) / 1000;
	printf("Finished perft in %f seconds\n", seconds);
	printf("NPS: %f\n", size / seconds);
	return size;
}

void moveGeneratorValidator()
{
	GameState testPos;
	loadFEN(&testPos, PERFT_POSITION_1);
	if (runPerft(7, &testPos) != 3195901860ULL)
	{
		fprintf(stderr, "PERFT 1 FAILURE\n");
		exit(1);
	}
	
	loadFEN(&testPos, PERFT_POSITION_2);
	if (runPerft(6, &testPos) != 8031647685ULL)
	{
		fprintf(stderr, "PERFT 2 FAILURE\n");
		exit(1);
	}
	
	loadFEN(&testPos, PERFT_POSITION_3);
	if (runPerft(8, &testPos) != 3009794393ULL)
	{
		fprintf(stderr, "PERFT 3 FAILURE\n");
		exit(1);
	}
	
	loadFEN(&testPos, PERFT_POSITION_4);
	if (runPerft(6, &testPos) != 706045033ULL)
	{
		fprintf(stderr, "PERFT 4 FAILURE\n");
		exit(1);
	}
	
	loadFEN(&testPos, PERFT_POSITION_4B);
	if (runPerft(6, &testPos) != 706045033ULL)
	{
		fprintf(stderr, "PERFT 4B FAILURE\n");
		exit(1);
	}
	
	loadFEN(&testPos, PERFT_POSITION_5);
	if (runPerft(5, &testPos) != 89941194ULL)
	{
		fprintf(stderr, "PERFT 5 FAILURE\n");
		exit(1);
	}
	
	loadFEN(&testPos, PERFT_POSITION_6);
	if (runPerft(6, &testPos) != 6923051137ULL)
	{
		fprintf(stderr, "PERFT 6 FAILURE\n");
		exit(1);
	}
	printf("PERFT PASSED\n");
}