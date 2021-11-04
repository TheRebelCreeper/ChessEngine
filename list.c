#include "list.h"

void scoreMoves(MoveList *moves, GameState *pos)
{
	int i;
	for (i = 0; i < moves->nextOpen; i++)
	{
		// Score captures
		if (moves->list[i].prop & IS_CAPTURE)
		{
			int offset = 6 * (pos->turn ^ 1);
			int victem = P;
			for (int j = P + offset; j <=K + offset; j++)
			{
				if (get_square(pos->pieceBitboards[j], moves->list[i].dst))
				{
					victem = j - offset;
					break;
				}
			}
			offset = 6 * pos->turn;
			moves->list[i].score = MVV_LVA_TABLE[moves->list[i].piece - offset][victem];
		}
		else
		{
			moves->list[i].score = moves->list.prop;
		}
	}
}