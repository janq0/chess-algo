#!/bin/sh

FILES='main.c gui.c gui.h chess.c chess.h bitboard.c bitboard.h'
FLAGS='-lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -Wall'

x86_64-w64-mingw32-gcc $FILES $FLAGS
