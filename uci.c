#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "uci.h"

// The code is to allow the engine to connect to GUI's via UCI protocol

Move parseMove(char *inputString)
{
	int src = (inputString[1] - '0' - 1) * 8 + (tolower(inputString[0]) - 'a');
	int dst = (inputString[3] - '0' - 1) * 8 + (tolower(inputString[2]) - 'a');
	
	//// Find move and play it
	//for (int i = 0; i < moveList.nextOpen; i++)
	//{
	//	Move move = moveList.list[i];
	//	if (move.prop & IS_PROMOTION)
	//	{
	//		
	//	}
	//	if (move.src == src && move.dst == dst && move.legal)
	//	{
	//		state = playMove(&state, move);
	//		found = 1;
	//		break;
	//	}
	//}
}