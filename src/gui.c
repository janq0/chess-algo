#include "gui.h"
#include <stdbool.h>
#include <stdio.h>
#include "bitboard.h"
#include "chess.h"
#include "defs.h"
#include "raylib.h"
#include "search.h"

static int square_size, window_size;
static Color square_colors[piececolor_cnt];
Texture2D piece_textures[piececolor_cnt][piecetype_cnt];
struct game game;
static int floating_piece_index = -1;
static Vector2 floating_piece_pos;

// Should be set to the size of the piece textures
static void
set_square_size(int value) {
    square_size = value;
    window_size = value * 8;
}

static void
set_square_colors(Color light, Color dark) {
    square_colors[PC_W] = light;
    square_colors[PC_B] = dark;
}

static void
load_textures(void) {
    printf("%s", GetWorkingDirectory());
    piece_textures[PC_W][P] = LoadTexture("img/wp96.png");
    piece_textures[PC_B][P] = LoadTexture("img/bp96.png");
    piece_textures[PC_W][R] = LoadTexture("img/wr96.png");
    piece_textures[PC_B][R] = LoadTexture("img/br96.png");
    piece_textures[PC_W][N] = LoadTexture("img/wn96.png");
    piece_textures[PC_B][N] = LoadTexture("img/bn96.png");
    piece_textures[PC_W][B] = LoadTexture("img/wb96.png");
    piece_textures[PC_B][B] = LoadTexture("img/bb96.png");
    piece_textures[PC_W][Q] = LoadTexture("img/wq96.png");
    piece_textures[PC_B][Q] = LoadTexture("img/bq96.png");
    piece_textures[PC_W][K] = LoadTexture("img/wk96.png");
    piece_textures[PC_B][K] = LoadTexture("img/bk96.png");
}

static void
unload_textures(void) {
    for (int color = 0; color < piececolor_cnt; color++) {
        for (int pt = 0; pt < piecetype_cnt; pt++) {
            UnloadTexture(piece_textures[color][pt]);
        }
    }
}

static void
draw_board_squares(void) {
    for (int row = 0; row < 8; row++) {
        for (int column = 0; column < 8; column++) {
            Color color = square_colors[(row + column) % 2];
            int posX = column * square_size;
            int posY = row * square_size;
            DrawRectangle(posX, posY, square_size, square_size, color);
        }
    }
}

static void
draw_piece(enum piececolor color, enum piecetype type, int i, bool value) {
    if (!value) {
        return;
    }
    Vector2 pos;
    if (floating_piece_index != i) {
        pos.x = (int)(i % 8) * square_size;
        pos.y = (int)(i / 8) * square_size;
    } else {
        int centering_shift = -square_size / 2;
        pos = GetMousePosition();
        pos.x += centering_shift;
        pos.y += centering_shift;
    }
    DrawTextureV(piece_textures[color][type], pos, WHITE);
}

static void
draw_pieces(void) {
    for (int pt = 0; pt < piecetype_cnt; pt++) {
        for (int pc = 0; pc < piececolor_cnt; pc++) {
            bitboard bb = game.pcs[pc][pt];
            for (int i = 0; i < 64; i++) {
                draw_piece(pc, pt, i, bb_get(bb, i));
            }
        }
    }
}

static void
draw_board(void) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    draw_board_squares();
    draw_pieces();
    EndDrawing();
}

static int
board_index_by_pos(Vector2 pos) {
    int row = pos.y / square_size;
    int column = pos.x / square_size;
    return 8 * row + column;
}

static void
handle_mouse(void) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        floating_piece_pos = GetMousePosition();
        floating_piece_index = board_index_by_pos(floating_piece_pos);
    }
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        int from = floating_piece_index;
        int to = board_index_by_pos(GetMousePosition());
        struct move move = infered_game_move(&game, from, to);
        if (game.turn == PC_W && move_is_leg(&game, move)) {
            ui_game_move(&game, move);
            struct move m = best_move(&game);
            ui_game_move(&game, m);
        }
        //printf("eval: %d\n", eval(&game));
        floating_piece_index = -1;
    }
    if (floating_piece_index != -1) {
        floating_piece_pos = GetMousePosition();
    }
}

void
start_gui(void) {
    game = init_game();
    set_square_size(96);
    InitWindow(window_size, window_size, "chess-algo");
    load_textures();
    set_square_colors((Color){244, 204, 172, 255}, (Color){199, 141, 110, 255});
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        draw_board();
        handle_mouse();
    }
    unload_textures();
    CloseWindow();
}
