#!/bin/bash
gcc *.c *.h -Wall -g -O3 -Drandom=rand -fopenmp -o engine.exe

