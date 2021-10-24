#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "movegen.h"
#include "search.h"
#include "perft.h"
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

// Example command
// position fen r2qkbnr/ppp2ppp/2np4/4N3/2B1P3/2N5/PPPP1PPP/R1BbK2R w KQkq - 0 6 moves c4f7 e8e7 c3d5
void parsePosition(char *line, GameState *pos)
{
	char *temp;
	
    line += 9;                     // Start the line after the word "position"
    temp = line;
    
    if (strncmp(line, "startpos", 8) == 0)
	{
        loadFEN(pos, STARTING_FEN);
	}
    else if (strncmp(line, "fen", 3) == 0)
    {
		temp += 4;                 // Start temp after the word "fen"
		loadFEN(pos, temp);
    }
	else
	{
		loadFEN(pos, STARTING_FEN);
	}
    
    temp = strstr(line, "moves");
    if (temp != NULL)
    {
		int size;
		MoveList moveList;
        temp += 6;                 // Length of "moves "
        
        while(*temp)
        {
			moveList = generateMoves(pos, &size);
            int idx = parseMove(temp, &moveList);
            
            if (idx == -1)
			{
                break;
			}
            
			*pos = playMove(pos, moveList.list[idx]);
			// Increment temp till the next move
            while (*temp && *temp != ' ')
			{
				temp++;
			}
            temp++;
        }
    }
}

void parseGo(char *line, GameState *pos)
{
	Move bestMove;
	int score;
	int depth = 6;
	char *temp;
	
    line += 3;                     // Start the line after the word "go"
    temp = line;
    
	if (strncmp(line, "perft", 5) == 0)
	{
		depth = atoi(temp + 6);
        runPerft(depth, pos);
		return;
	}
	
	temp = strstr(line, "depth");
    if (temp != NULL)
    {
		depth = atoi(temp + 6);
    }
	
	bestMove = search(depth, pos, &score);
	int mated = 0;
	if (score > MAX_PLY_CHECKMATE)
	{
		score = CHECKMATE - score;
		mated = 1;
	}
	else if (score < -MAX_PLY_CHECKMATE)
	{
		score = -CHECKMATE - score;
		mated = 1;
	}
	
	printf("info depth %d ", depth);
	printf("score %s %d ", (mated) ? "mate" : "cp", score);
	printf("pv %s%s%s\n", squareNames[bestMove.src], squareNames[bestMove.dst], (bestMove.prop & IS_PROMOTION) ? pieceNotation[bestMove.special] : "");
	printf("bestmove %s%s%s\n", squareNames[bestMove.src], squareNames[bestMove.dst], (bestMove.prop & IS_PROMOTION) ? pieceNotation[bestMove.special] : "");
	
}

void uciLoop()
{
	GameState pos;
	char buf[2048];
	parsePosition("position startpos", &pos);
	while (1)
	{
		memset(buf, 0, sizeof(buf));
		fflush(stdout);
		
		int c;
		
		
		if (fgets(buf, sizeof(buf), stdin) == NULL)
		{
			exit(1);
		}
		
		while ((c = getchar()) != '\n' && c != EOF)
		
		// Engine should respond with "readyok\n"
		if (strncmp(buf, "isready", 7) == 0)
		{
			printf("readyok\n");
			continue;
		}
		
		if (strncmp(buf, "position", 8) == 0)
		{
			parsePosition(buf, &pos);
		}
		else if (strncmp(buf, "ucinewgame", 10) == 0)
		{
			parsePosition("position startpos", &pos);
		}
		else if (strncmp(buf, "go", 2) == 0)
		{
			parseGo(buf, &pos);
		}
		else if (strncmp(buf, "d\n", 2) == 0)
		{
			printBoard(pos);
		}			
		else if (strncmp(buf, "quit", 4) == 0)
		{
			return;
		}
		else if (strncmp(buf, "uci", 3) == 0)
		{
			// Print engine info
			printf("id name Saxton\n");
			printf("id author Aaron Lampert\n");
			printf("uciok\n");
		}
	}
}