#!/bin/bash
gcc *.c *.cpp *.h -Wall -g -O3 -Drandom=rand -D__USE_MINGW_ANSI_STDIO=1 -fopenmp -o engine.exe -static

