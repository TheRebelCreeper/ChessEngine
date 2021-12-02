#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
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
		promotionPiece = GET_MOVE_PROMOTION(move);

		if (GET_MOVE_SRC(move) == src && GET_MOVE_DST(move) == dst)
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
	historyIndex = 0;
	memset(posHistory, 0, sizeof(posHistory));

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
		int size, legal;
		MoveList moveList;
		temp += 6;                 // Length of "moves "

		while(*temp)
		{
			moveList = generateMoves(pos, &size);
			int idx = parseMove(temp, &moveList);
			int piece = GET_MOVE_PIECE(moveList.list[idx]);

			if (idx == -1)
			{
				break;
			}

			GameState tempState = playMove(pos, moveList.list[idx], &legal);
			if (!legal)
			{
				break;
			}

			// If the move is a pawn push or capture, reset history list
			if (GET_MOVE_CAPTURED(moveList.list[idx]) || piece == P || piece == p)
			{
				historyIndex = 0;
			}

			// Add the legal move to history
			posHistory[historyIndex] = tempState.key;
			historyIndex++;

			*pos = tempState;
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
	int depth = -1, movestogo = 30,movetime = -1;
	int wtime = -1, btime = -1, time = -1, inc = 0;

	SearchInfo info;
	info.stopped = 0;
	info.timeset = 0;
	char *temp;
	
	line += 3;                     // Start the line after the word "go"
	temp = line;

	if (strncmp(line, "perft", 5) == 0)
	{
		info.depth = atoi(temp + 6);
		runPerft(info.depth, pos);
		return;
	}

	temp = strstr(line, "infinite");
	if (temp != NULL)
	{
		;
	}

	temp = strstr(line, "winc");
	if (temp != NULL && pos->turn == WHITE)
	{
		inc = atoi(temp + 5);
	}

	temp = strstr(line, "binc");
	if (temp != NULL && pos->turn == BLACK)
	{
		inc = atoi(temp + 5);
	}


	temp = strstr(line, "wtime");
	if (temp != NULL && pos->turn == WHITE)
	{
		wtime = atoi(temp + 6);
		time = wtime;
	}

	temp = strstr(line, "btime");
	if (temp != NULL && pos->turn == BLACK)
	{
		btime = atoi(temp + 6);
		time = btime;
	}

	temp = strstr(line, "movestogo"); 
	if (temp != NULL)
	{
		movestogo = atoi(temp + 10);
	} 

	temp = strstr(line, "movetime");
	if (temp != NULL)
	{
		movetime = atoi(temp + 9);
	}

	temp = strstr(line, "depth");
	if (temp != NULL)
	{
		depth = atoi(temp + 6);
	}

	if(movetime != -1)
	{
		time = movetime;
		movestogo = 1;
	}
	
	info.starttime = GetTimeMs();
	
	if(time != -1)
	{
		info.timeset = 1;
		if (time < inc)
			inc = 0;
		time /= movestogo;
		time -= 50;
		if (pos->turn == WHITE && time - 10000 > btime)
		{
			time += (time - btime / 4);
		}
		else if (pos->turn == BLACK && time - 10000 > wtime)
		{
			time += (time - wtime / 4);
		}
		if (time < 0)
			time = 0;
		info.stoptime = info.starttime + time + inc + 1;
	} 
	
	if(depth == -1)
	{
		info.depth = MAX_PLY - 1;
	}
	else
	{
		info.depth = depth;
	}
	
	search(pos, &info);
}

void uciLoop()
{
	GameState pos;
	
	// Needed to correctly output to GUI program, not sure why
	setbuf(stdin, NULL);
	setbuf(stdout, NULL);
	
	char buf[2048];
	parsePosition("position startpos", &pos);
	while (1)
	{
		memset(buf, 0, sizeof(buf));
		fflush(stdout);
		
		if (fgets(buf, sizeof(buf), stdin) == NULL)
		{
			exit(1);
		}
		
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
		else if (strncmp(buf, "setoption name Threads", 22) == 0)
		{
			NUM_THREADS = atoi(buf + 28);
			if (NUM_THREADS <= 0 || NUM_THREADS > 12)
			{
				NUM_THREADS = 1;
			}
		}
		else if (strncmp(buf, "uci", 3) == 0)
		{
			// Print engine info
			printf("id name Saxton\n");
			printf("id author Aaron Lampert\n");
			printf("option name Threads type spin default 1 min 1 max 12\n");
			printf("uciok\n");
		}
	}
}