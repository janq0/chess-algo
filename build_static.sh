#!/bin/sh
# NOTE: Build on a distro with an older version of GLIBC, such as Ubuntu LTS
# to make sure that the executable will run on most distros.

FILES='src/main.c src/gui.c src/gui.h src/chess.c src/chess.h src/bitboard.c src/bitboard.h src/search.c src/search.h'
FLAGS='./raylib/src/libraylib.a -I./raylib/src -lGL -lm -lpthread -ldl -lrt -lX11 -Wall -O3'

gcc $FILES $FLAGS
