#!/bin/bash
gcc *.c *.h -pg -Drandom=rand -fopenmp -o engine.exe

