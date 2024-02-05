#include <stdio.h>
#include "chess.h"
#include "bitboard.h"
#include "defs.h"

unsigned long long
perft(struct game *g, int depth) {
    if (depth == 0) {
        return 1ULL;
    }
    int nodes = 0;
    struct game sim;
    struct movelist ml = leg_moves(g);
    for (int i = 0; i < ml.count; i++) {
        sim = *g;
        game_move_no_leg_check(&sim, ml.moves[i]);
        nodes += perft(&sim, depth - 1);
    }
    return nodes;
}

int main(void) {
    gen_bbs();
    struct game g = init_game();
    printf("%d: %llu\n", 5, perft(&g, 5));
    return 0;
}
