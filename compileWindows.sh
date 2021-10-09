#!/bin/bash
gcc *.c *.h -g -O3 -Drandom=rand -fopenmp -o engine.exe

