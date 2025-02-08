#!/bin/bash

DEFINES="-Drandom=rand -D__USE_MINGW_ANSI_STDIO=1 -DUSE_AVX2 -mavx2 -DUSE_SSE41 -msse4.1 -DUSE_SSSE3 -mssse3 -DUSE_SSE2 -msse2 -DUSE_SSE -msse"
FLAGS="-Wall -fstrict-aliasing -fno-exceptions -Wno-unused-variable -Wno-unused-result -Wno-unused-but-set-variable -Wno-maybe-uninitialized -Ofast -fomit-frame-pointer"

gcc *.c *.h ${FLAGS} ${DEFINES} -fcommon -o Saxton.exe
