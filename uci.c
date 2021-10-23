#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "uci.h"

// The code is to allow the engine to connect to GUI's via UCI protocol

int parseMove(char *inputString, MoveList *moveList)
{
	int src = (inputString[1] - '0' - 1) * 8 + (tolower(inputString[0]) - 'a');
	int dst = (inputString[3] - '0' - 1) * 8 + (tolower(inputString[2]) - 'a');
	int promotionPiece = 0;
	
	for (int i = 0; i < moveList->nextOpen; i++)
	{
		Move move = moveList->list[i];
		
		if (move.prop & IS_PROMOTION)
		{
			promotionPiece = move.special;
		}
		
		if (move.src == src && move.dst == dst && move.legal)
		{
			if (promotionPiece)
			{
				if (promotionPiece == Q && tolower(inputString[4]) == 'q')
				{
					return i;
				}
				if (promotionPiece == N && tolower(inputString[4]) == 'n')
				{
					return i;
				}
				if (promotionPiece == B && tolower(inputString[4]) == 'b')
				{
					return i;
				}
				if (promotionPiece == R && tolower(inputString[4]) == 'r')
				{
					return i;
				}
				continue;
			}
			return i;
		}
	}
	return -1;
}