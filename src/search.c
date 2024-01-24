#include "search.h"
#include <limits.h>
#include <stdio.h>
#include "bitboard.h"
#include "chess.h"
#include "defs.h"

const int pc_weights[piecetype_cnt] = {
    [K] = 2000, [Q] = 90, [R] = 50, [B] = 30, [N] = 30, [P] = 10};
const int mobility_weight = 1;

int
mob_score(struct game* g) {
    int result = 0;
    result += pleg_moves(g).count;
    g->turn = !g->turn;
    result -= pleg_moves(g).count;
    g->turn = !g->turn;
    return result;
}

int
eval(struct game* g) {
    if (!g->active) {
        return g->turn ? -9999 : 9999;
    }
    int result = 0;
    bitboard rem;
    for (int i = 0; i < piecetype_cnt; i++) {
        int cnt = 0;
        rem = g->pcs[PC_W][i];
        while (rem != 0) {
            BB_MUT_POP(rem, bb_ls1b(rem));
            cnt++;
        }
        rem = g->pcs[PC_B][i];
        while (rem != 0) {
            BB_MUT_POP(rem, bb_ls1b(rem));
            cnt--;
        }
        result += cnt * pc_weights[i];
    }
    return (result + mob_score(g)) * (g->turn ? -1 : 1);
}

int
negamax(struct game* g, int depth) {
    if (depth == 0 || !g->active) {
        return eval(g);
    }
    int max = INT_MIN, cur;
    struct game sim;
    struct movelist ml = leg_moves(g);
    for (int i = 0; i < ml.count; i++) {
        sim = *g;
        game_move(&sim, ml.moves[i]);
        cur = -negamax(&sim, depth - 1);
        if (cur > max) {
            max = cur;
        }
    }
    return max;
}

struct move
best_move(struct game* g) {
    struct movelist ml = leg_moves(g);
    struct move bestmv;
    struct game sim;
    int max = INT_MIN, cur;
    for (int i = 0; i < ml.count; i++) {
        sim = *g;
        game_move(&sim, ml.moves[i]);
        cur = -negamax(&sim, 2);
        if (cur > max) {
            max = cur;
            printf(".");
            bestmv = ml.moves[i];
        }
    }
    return bestmv;
}