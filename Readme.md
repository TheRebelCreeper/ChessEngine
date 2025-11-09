## Overview

Saxton is a chess engine written in C, which started being developed in 2021 as part of my senior project in university.

## Search Features

This engine features many techniques in order to calculate both quickly and deeply.

* Fail-soft negamax
* Quiescence search
* PVS Search
* Transposition Table
* Iterative Deepening
* Internal Iterative Reduction
* Late Move Reductions
* Futility Pruning
* SEE pruning
* Reverse Futility Pruning
* Null Move Pruning
* Razoring
* Mate Distance Pruning
* Check Extensions
* MVA-LVV Move Ordering
* History Heuristic Move Ordering
* Killer Move Heuristic

## TODO List

Features that have not been implemented yet, but are planned

* New NNUE
* TT in qsearch
* Aspiration Windows
* Lazy SMP
* Late Move Pruning
* Tunability
* Staged Move Generation

## Requirements

Requires the NNUE weights to be in same directory as executable.  
Only supports the file kept in the repo.

## The UCI protocol and available options

This engine uses the UCI protocol.

### Move format:

The move format is in long algebraic notation.  
A nullmove from the Engine to the GUI should be sent as 0000.  
Examples:  e2e4, e7e5, e1g1 (white short castling), e7e8q (for promotion)

### GUI to engine:

These are all the supported commands the engine gets from the interface.

* uci  
  tell engine to use the uci (universal chess interface),
  this will be sent once as a first command after program boot

* isready  
  this is used to synchronize the engine with the GUI. When the GUI has sent a command or
  multiple commands that can take some time to complete,
  this command can be used to wait for the engine to be ready again or
  to ping the engine to find out if it is still alive.

* ucinewgame  
  this is sent to the engine when the next search (started with "position" and "go") will be from
  a different game.

* position [fen <fenstring> | startpos ]  moves <move1> .... <movei>  
  set up the position described in fenstring on the internal board and
  play the moves on the internal chess board.
  if the game was played from the start position the string "startpos" will be sent.

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

* quit  
  quit the program as soon as possible
