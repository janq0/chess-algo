#include "search.h"
#include <limits.h>
#include "bitboard.h"
#include "chess.h"
#include "defs.h"

#define MIN(i, j) (((i) < (j)) ? (i) : (j))
#define MAX(i, j) (((i) > (j)) ? (i) : (j))

const int pc_weights[piecetype_cnt] = {
    [K] = 2000, [Q] = 90, [R] = 50, [B] = 30, [N] = 30, [P] = 10};
const int mobility_weight = 1;

int
mob_score(struct game* g) {
    int result = 0;
    result -= pleg_moves(g).count;
    g->turn = !g->turn;
    result += pleg_moves(g).count;
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
negamax(struct game* g, int depth, int a, int b) {
    if (depth == 0 || !g->active) {
        return eval(g);
    }
    int val = INT_MIN + 1;
    struct game sim;
    struct movelist ml = leg_moves(g);
    for (int i = 0; i < ml.count; i++) {
        sim = *g;
        game_move_no_leg_check(&sim, ml.moves[i]);
        val = MAX(val, -negamax(&sim, depth - 1, -b, -a));
        a = MAX(a, val);
        if (b <= a) {
            break;
        }
    }
    return val;
}

struct move
best_move(struct game* g) {
    struct movelist ml = leg_moves(g);
    struct move bestmv;
    struct game sim;
    int max = INT_MIN + 1, cur;
    for (int i = 0; i < ml.count; i++) {
        sim = *g;
        game_move_no_leg_check(&sim, ml.moves[i]);
        cur = -negamax(&sim, 3, INT_MIN + 1, INT_MAX);
        if (cur > max) {
            max = cur;
            bestmv = ml.moves[i];
        }
    }
    return bestmv;
}
