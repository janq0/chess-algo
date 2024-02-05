#!/bin/sh

FILES='src/perft.c src/gui.c src/gui.h src/chess.c src/chess.h src/bitboard.c src/bitboard.h src/search.c src/search.h'
FLAGS='-g -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -Wall'

cc $FILES $FLAGS
