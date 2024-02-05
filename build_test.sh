#!/bin/sh

# FILES='src/gui.c src/gui.h src/chess.c src/chess.h src/bitboard.c src/bitboard.h src/test.c'
FILES='src/test.c'
FLAGS='-lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -Wall'

cc $FILES $FLAGS
