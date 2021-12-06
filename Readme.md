### Senior Project/CSC 480 Project
Created by Aaron Lampert  
CSC 480 Professor: Dr. Rodrigo Canaan  
This would not have been possible with referencing both Stockfish and Chessprogramming wiki

### Example of Results
Bot account: https://lichess.org/@/SaxtonEngine  
Nice game: https://lichess.org/J5dYSKMx  

## Requirements
Requires OMP to compile.  
Requires the NNUE weights to be in same directory as executable.  
Only supports the file kept in the repo.  

## Usage
./Saxton.exe or ./Saxton depending on whether or not running on windows

## The UCI protocol and available options

Move format:
------------

The move format is in long algebraic notation.  
A nullmove from the Engine to the GUI should be sent as 0000.  
Examples:  e2e4, e7e5, e1g1 (white short castling), e7e8q (for promotion)  



GUI to engine:
--------------

These are all the command the engine gets from the interface.

* uci  
	tell engine to use the uci (universal chess interface),
	this will be sent once as a first command after program boot
	to tell the engine to switch to uci mode.
	After receiving the uci command the engine must identify itself with the "id" command
	and send the "option" commands to tell the GUI which engine settings the engine supports if any.
	After that the engine should send "uciok" to acknowledge the uci mode.
	If no uciok is sent within a certain time period, the engine task will be killed by the GUI.

* isready  
	this is used to synchronize the engine with the GUI. When the GUI has sent a command or
	multiple commands that can take some time to complete,
	this command can be used to wait for the engine to be ready again or
	to ping the engine to find out if it is still alive.
	E.g. this should be sent after setting the path to the tablebases as this can take some time.
	This command is also required once before the engine is asked to do any search
	to wait for the engine to finish initializing.
	This command must always be answered with "readyok" and can be sent also when the engine is calculating
	in which case the engine should also immediately answer with "readyok" without stopping the search.

* ucinewgame  
   this is sent to the engine when the next search (started with "position" and "go") will be from
   a different game. This can be a new game the engine should play or a new game it should analyse but
   also the next position from a testsuite with positions only.
   If the GUI hasn't sent a "ucinewgame" before the first "position" command, the engine shouldn't
   expect any further ucinewgame commands as the GUI is probably not supporting the ucinewgame command.
   So the engine should not rely on this command even though all new GUIs should support it.
   As the engine's reaction to "ucinewgame" can take some time the GUI should always send "isready"
   after "ucinewgame" to wait for the engine to finish its operation.
   
* position [fen <fenstring> | startpos ]  moves <move1> .... <movei>  
	set up the position described in fenstring on the internal board and
	play the moves on the internal chess board.
	if the game was played  from the start position the string "startpos" will be sent
	Note: no "new" command is needed. However, if this position is from a different game than
	the last position sent to the engine, the GUI should have sent a "ucinewgame" inbetween.

* d  
	Displays the position using ASCII

* go  
	start calculating on the current position set up with the "position" command.
	There are a number of commands that can follow this command, all will be sent in the same string.
	If one command is not sent its value should be interpreted as it would not influence the search.
	* wtime <x>
		white has x msec left on the clock
	* btime <x>
		black has x msec left on the clock
	* winc <x>
		white increment per move in mseconds if x > 0
	* binc <x>
		black increment per move in mseconds if x > 0
	* movestogo <x>
      there are x moves to the next time control,
		this will only be sent if x > 0,
		if you don't get this and get the wtime and btime it's sudden death
	* depth <x>
		search x plies only.
	* movetime <x>
		search exactly x mseconds
	* infinite
		search until the "stop" command. Do not exit the search without being told so in this mode!
	* perft
		runs a test of the move generator. Must include a depth aswell
    
* stop  
	stop calculating as soon as possible,
	don't forget the "bestmove" and possibly the "ponder" token when finishing the search

* quit  
	quit the program as soon as possible


Engine to GUI:
--------------

* id
	* name <x>
		this must be sent after receiving the "uci" command to identify the engine,
		e.g. "id name Shredder X.Y\n"
	* author <x>
		this must be sent after receiving the "uci" command to identify the engine,
		e.g. "id author Stefan MK\n"

* uciok  
	Must be sent after the id and optional options to tell the GUI that the engine
	has sent all infos and is ready in uci mode.

* readyok  
	This must be sent when the engine has received an "isready" command and has
	processed all input and is ready to accept new commands now.
	It is usually sent after a command that can take some time to be able to wait for the engine,
	but it can be used anytime, even when the engine is searching,
	and must always be answered with "isready".

* bestmove <move1> [ ponder <move2> ]  
	the engine has stopped searching and found the move <move> best in this position.
	the engine can send the move it likes to ponder on. The engine must not start pondering automatically.
	this command must always be sent if the engine stops searching, also in pondering mode if there is a
	"stop" command, so for every "go" command a "bestmove" command is needed!
	Directly before that the engine should send a final "info" command with the final search information,
	the the GUI has the complete statistics about the last search.
	      
* info  
	the engine wants to send information to the GUI. This should be done whenever one of the info has changed.
	The engine can send only selected infos or multiple infos with one info command,
	e.g. "info currmove e2e4 currmovenumber 1" or
	     "info depth 12 nodes 123456 nps 100000".
	Also all infos belonging to the pv should be sent together
	e.g. "info depth 2 score cp 214 time 1242 nodes 2124 nps 34928 pv e2e4 e7e5 g1f3"
	I suggest to start sending "currmove", "currmovenumber", "currline" and "refutation" only after one second
	to avoid too much traffic.
	Additional info:
	* depth <x>
		search depth in plies
	* time <x>
		the time searched in ms, this should be sent together with the pv.
	* nodes <x>
		x nodes searched, the engine should send this info regularly
	* pv <move1> ... <movei>
		the best line found
	* score
		* cp <x>
			the score from the engine's point of view in centipawns.
		* mate <y>
			mate in y moves, not plies.
			If the engine is getting mated use negative values for y.
	* nps <x>
		x nodes per second searched, the engine should send this info regularly
