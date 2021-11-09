#include <stdio.h>
#include "list.h"

// Prints psuedolegal moves
void printMoveList(MoveList *list)
{
	for (int i = 0; i < list->nextOpen; i++)
	{
		Move temp = list->list[i];
		if (!(temp.prop & IS_PROMOTION))
		{
			printf("move: %s%s score: %d\n", squareNames[temp.src], squareNames[temp.dst], temp.score);
		}
		else
		{
			printf("move: %s%s%s score: %d\n", squareNames[temp.src], squareNames[temp.dst], pieceNotation[temp.special], temp.score);
		}
	}
}