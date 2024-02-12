emcc -o game.html src/main.c src/gui.c src/chess.c src/bitboard.c src/search.c -Os -Wall web_libraylib.a -I. -Isrc -L. -s USE_GLFW=3 -s ASYNCIFY -DPLATFORM_WEB
