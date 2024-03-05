#!/bin/sh
# NOTE: Build on a distro with an older version of GLIBC, such as Ubuntu LTS
# to make sure that the executable will run on most distros.

FILES='src/main.c src/gui.c src/gui.h src/chess.c src/chess.h src/bitboard.c src/bitboard.h src/search.c src/search.h'
FLAGS='./raylib-win/src/libraylib.a -I./raylib-win/src -lopengl32 -lgdi32 -lwinmm -Wall -O3'

gcc $FILES -Wall -std=c99 -D_DEFAULT_SOURCE -O3 -I./src -I./raylib-w/src -I./raylib-w/src/external -L./src -L./raylib-w/src -L./raylib-w/src -lraylib -lopengl32 -lgdi32 -lwinmm
