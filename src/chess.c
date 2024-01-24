#include "chess.h"
#include <alloca.h>
#include <stdbool.h>
#include <stdio.h>
#include "bitboard.h"
#include "defs.h"

void
print_move(struct move m) {
    char color, piece, type;
    switch (m.color) {
        case PC_W: color = 'w'; break;
        case PC_B: color = 'b'; break;
        default: color = '?';
    }
    switch (m.piece) {
        case P: piece = 'p'; break;
        case N: piece = 'n'; break;
        case R: piece = 'r'; break;
        case B: piece = 'b'; break;
        case Q: piece = 'q'; break;
        case K: piece = 'k'; break;
        default: piece = '?';
    }
    switch (m.type) {
        case MT_ATTACK: type = 'a'; break;
        case MT_PROMOTION: type = 'p'; break;
        case MT_QUIET: type = 'q'; break;
        case MT_CASTLE: type = 'c'; break;
    }
    printf("%c%c from %d to %d (%c)", color, piece, m.from, m.to, type);
}

bool
cmp_move(struct move m1, struct move m2) {
    return m1.color == m2.color && m1.from == m2.from && m1.to == m2.to
           && m1.piece == m2.piece;
}

struct game
init_game(void) {
    struct game g;
    g.turn = PC_W;

    g.pcs[PC_W][P] = 0x000000000000FF00;
    g.pcs[PC_B][P] = 0x00FF000000000000;
    g.pcs[PC_W][R] = 0x0000000000000081;
    g.pcs[PC_B][R] = 0x8100000000000000;
    g.pcs[PC_W][N] = 0x0000000000000042;
    g.pcs[PC_B][N] = 0x4200000000000000;
    g.pcs[PC_W][B] = 0x0000000000000024;
    g.pcs[PC_B][B] = 0x2400000000000000;
    g.pcs[PC_W][Q] = 0x0000000000000010;
    g.pcs[PC_B][Q] = 0x1000000000000000;
    g.pcs[PC_W][K] = 0x0000000000000008;
    g.pcs[PC_B][K] = 0x0800000000000000;

    for (int c = 0; c < piececolor_cnt; c++) {
        g.occ[c] = 0ULL;
        for (int t = 0; t < piecetype_cnt; t++) {
            g.occ[c] |= g.pcs[c][t];
        }
    }
    g.nonocc = ~(g.occ[PC_B] | g.occ[PC_W]);
    g.enp_targets[PC_W] = 0ULL;
    g.enp_targets[PC_B] = 0ULL;
    g.active = true;
    g.castling_rights[0] = true;
    g.castling_rights[1] = true;
    g.castling_rights[2] = true;
    g.castling_rights[3] = true;

    return g;
}

struct move
infered_game_move(struct game* g, int from, int to) {
    struct move m;
    m.color = g->turn;
    m.from = from;
    m.to = to;
    for (int pt = 0; pt < piecetype_cnt; pt++) {
        if (bb_get(g->pcs[g->turn][pt], from)) {
            m.piece = (enum piecetype)pt;
            m.type = (bb_set(0ULL, m.to) & g->nonocc) ? MT_QUIET : MT_ATTACK;
            if (m.piece == P && (m.to / 8 == 0 || m.to / 8 == 7)) {
                m.type = MT_PROMOTION;
                char input;
                printf("Speficy the promotion [n/b/r/q]: ");
                scanf("%c", &input);
                switch (input) {
                    case 'n': m.promotion = N; break;
                    case 'b': m.promotion = B; break;
                    case 'r': m.promotion = R; break;
                    case 'q': m.promotion = Q; break;
                    default: m.promotion = Q;
                }
            }
            if (m.piece == K && (m.to == m.from - 2 || m.to == m.from + 2)) {
                m.type = MT_CASTLE;
            }
            return m;
        }
    }

    m.piece = PT_INVALID;
    m.promotion = PT_INVALID;
    return m;
}

static bitboard
P_push_pleg_targets(bitboard nonocc, bitboard pawns, enum piececolor pc) {
    bitboard result = 0ULL;
    if (pc == PC_W) {
        result |= (pawns << 8) & nonocc;
        result |= (result << 8) & nonocc & rank4_bb;
    } else {
        result |= (pawns >> 8) & nonocc;
        result |= (result >> 8) & nonocc & rank5_bb;
    }
    return result;
}

static bitboard
P_atk_pleg_targets(bitboard targets, enum piececolor pc, int sqr) {
    return P_atk_pattern[pc][sqr] & targets;
}

static bitboard
N_pleg_targets(bitboard friendly_pieces, int sqr) {
    return N_pattern[sqr] & ~friendly_pieces;
}

static bitboard
K_pleg_targets(bitboard friendly_pieces, int sqr) {
    return K_pattern[sqr] & ~friendly_pieces;
}

static bitboard
sliding_ray(bitboard occ[2], enum piececolor pc, int row0, int col0, int drow,
            int dcol) {
    bitboard result = 0ULL;
    for (int row = row0 + drow, col = col0 + dcol;
         0 <= row && row < 8 && 0 <= col && col < 8; row += drow, col += dcol) {
        bitboard move = bb_set(0ULL, 8 * row + col);
        if (move & occ[pc]) {
            break;
        } else if (move & occ[!pc]) {
            result |= move;
            break;
        }
        result |= move;
    }
    return result;
}

static bitboard
R_pleg_targets(bitboard occ[2], enum piececolor pc, int sqr) {
    bitboard result = 0ULL;
    int row = sqr / 8, col = sqr % 8;
    result |= sliding_ray(occ, pc, row, col, 1, 0);
    result |= sliding_ray(occ, pc, row, col, 0, 1);
    result |= sliding_ray(occ, pc, row, col, -1, 0);
    result |= sliding_ray(occ, pc, row, col, 0, -1);
    return result;
}

static bitboard
B_pleg_targets(bitboard occ[2], enum piececolor pc, int sqr) {
    bitboard result = 0ULL;
    int row = sqr / 8, col = sqr % 8;
    result |= sliding_ray(occ, pc, row, col, 1, 1);
    result |= sliding_ray(occ, pc, row, col, 1, -1);
    result |= sliding_ray(occ, pc, row, col, -1, 1);
    result |= sliding_ray(occ, pc, row, col, -1, -1);
    return result;
}

static bitboard
Q_pleg_targets(bitboard occ[2], enum piececolor pc, int sqr) {
    return B_pleg_targets(occ, pc, sqr) | R_pleg_targets(occ, pc, sqr);
}

struct movelist
pleg_moves(struct game* g) {
    struct movelist result = {{0}, 0};
    bitboard from_rem = 0ULL, to_rem = 0ULL;
    int to_i = 0, from_i = 0;
    to_rem = P_push_pleg_targets(g->nonocc, g->pcs[g->turn][P], g->turn);
    while (to_rem != 0ULL) {
        to_i = bb_ls1b(to_rem);
        BB_MUT_POP(to_rem, to_i);
        int push = g->turn == PC_W ? -8 : 8;
        if (bb_set(0ULL, to_i - 2 * push) & g->pcs[g->turn][P]) {
            from_i = to_i - 2 * push;
        } else {
            from_i = to_i - push;
        }
        result.moves[result.count] =
            (struct move){from_i, to_i, g->turn, P, PT_INVALID, MT_QUIET};
        result.count++;
    }

    from_rem = g->pcs[g->turn][P];
    while (from_rem != 0ULL) {
        from_i = bb_ls1b(from_rem);
        BB_MUT_POP(from_rem, from_i);
        to_rem = P_atk_pleg_targets(g->occ[!g->turn] | g->enp_targets[!g->turn],
                                    g->turn, from_i);
        while (to_rem != 0ULL) {
            to_i = bb_ls1b(to_rem);
            BB_MUT_POP(to_rem, to_i);
            result.moves[result.count] =
                (struct move){from_i, to_i, g->turn, P, PT_INVALID, MT_ATTACK};
            result.count++;
        }
    }

    from_rem = g->pcs[g->turn][N];
    while (from_rem != 0ULL) {
        from_i = bb_ls1b(from_rem);
        BB_MUT_POP(from_rem, from_i);
        to_rem = N_pleg_targets(g->occ[g->turn], from_i);
        while (to_rem != 0ULL) {
            to_i = bb_ls1b(to_rem);
            BB_MUT_POP(to_rem, to_i);
            result.moves[result.count] =
                (struct move){from_i, to_i, g->turn, N, PT_INVALID, MT_ATTACK};
            result.count++;
        }
    }

    from_rem = g->pcs[g->turn][K];
    while (from_rem != 0ULL) {
        from_i = bb_ls1b(from_rem);
        BB_MUT_POP(from_rem, from_i);
        to_rem = K_pleg_targets(g->occ[g->turn], from_i);
        while (to_rem != 0ULL) {
            to_i = bb_ls1b(to_rem);
            BB_MUT_POP(to_rem, to_i);
            result.moves[result.count] =
                (struct move){from_i, to_i, g->turn, K, PT_INVALID, MT_ATTACK};
            result.count++;
        }
    }

    from_rem = g->pcs[g->turn][R];
    while (from_rem != 0ULL) {
        from_i = bb_ls1b(from_rem);
        BB_MUT_POP(from_rem, from_i);
        to_rem = R_pleg_targets(g->occ, g->turn, from_i);
        while (to_rem != 0ULL) {
            to_i = bb_ls1b(to_rem);
            BB_MUT_POP(to_rem, to_i);
            result.moves[result.count] =
                (struct move){from_i, to_i, g->turn, R, PT_INVALID, MT_ATTACK};
            result.count++;
        }
    }

    from_rem = g->pcs[g->turn][B];
    while (from_rem != 0ULL) {
        from_i = bb_ls1b(from_rem);
        BB_MUT_POP(from_rem, from_i);
        to_rem = B_pleg_targets(g->occ, g->turn, from_i);
        while (to_rem != 0ULL) {
            to_i = bb_ls1b(to_rem);
            BB_MUT_POP(to_rem, to_i);
            result.moves[result.count] =
                (struct move){from_i, to_i, g->turn, B, PT_INVALID, MT_ATTACK};
            result.count++;
        }
    }

    from_rem = g->pcs[g->turn][Q];
    while (from_rem != 0ULL) {
        from_i = bb_ls1b(from_rem);
        BB_MUT_POP(from_rem, from_i);
        to_rem = Q_pleg_targets(g->occ, g->turn, from_i);
        while (to_rem != 0ULL) {
            to_i = bb_ls1b(to_rem);
            BB_MUT_POP(to_rem, to_i);
            result.moves[result.count] =
                (struct move){from_i, to_i, g->turn, Q, PT_INVALID, MT_ATTACK};
            result.count++;
        }
    }
    return result;
}

bitboard
reachable_squares(struct game* g, enum piececolor color) {
    bitboard result = 0ULL, temp;
    result |= P_push_pleg_targets(g->nonocc, g->pcs[color][P], color);

    temp = g->pcs[color][P];
    int square;
    while (temp != 0) {
        square = bb_ls1b(temp);
        BB_MUT_POP(temp, square);
        result |= P_atk_pleg_targets(g->occ[!color] | g->enp_targets[!color],
                                     color, square);
    }

    temp = g->pcs[color][N];
    while (temp != 0) {
        square = bb_ls1b(temp);
        BB_MUT_POP(temp, square);
        result |= N_pleg_targets(g->occ[color], square);
    }

    temp = g->pcs[color][K];
    while (temp != 0) {
        square = bb_ls1b(temp);
        BB_MUT_POP(temp, square);
        result |= K_pleg_targets(g->occ[color], square);
    }

    temp = g->pcs[color][R];
    while (temp != 0) {
        square = bb_ls1b(temp);
        BB_MUT_POP(temp, square);
        result |= R_pleg_targets(g->occ, color, square);
    }

    temp = g->pcs[color][B];
    while (temp != 0) {
        square = bb_ls1b(temp);
        BB_MUT_POP(temp, square);
        result |= B_pleg_targets(g->occ, color, square);
    }

    temp = g->pcs[color][Q];
    while (temp != 0) {
        square = bb_ls1b(temp);
        BB_MUT_POP(temp, square);
        result |= Q_pleg_targets(g->occ, color, square);
    }

    return result;
}

// bool
// pawn_move_is_pseudolegal(struct game* g, struct game_move m) {
//     bitboard move = bb_set(0ULL, m.to);
//     print_move(m);
//     switch (m.type) {
//         case MT_ATTACK:;
//             bitboard move_mask = pawn_attack_bbs[m.color][m.from];
//             bitboard enemy_pieces = g->occupied[!m.color];
//             return (bool)(move & move_mask & enemy_pieces);
//         case MT_QUIET:;
//             bool same_col = (bool)(m.from % 8 == m.to % 8);
//             bitboard targets = pawn_push_targets(
//                 g->nonoccupied, g->pieces[m.color][PT_PAWN], m.color);
//             return same_col && (move & targets);
//         default: return false; // TODO: promo
//     }
// }

// bool
// knight_move_is_pseudolegal(struct game* g, struct game_move m) {
//     bitboard move = bb_set(0ULL, m.to);
//     bitboard moves = knight_pseudolegal_moves(g->occupied[m.color], m.from);
//     return (bool)(move & moves);
// }

// bool
// king_move_is_pseudolegal(struct game* g, struct game_move m) {
//     bitboard move = bb_set(0ULL, m.to);
//     bitboard moves = king_pseudolegal_moves(g->occupied[m.color], m.from);
//     return (bool)(move & moves);
// }

// bool
// rook_move_is_pseudolegal(struct game* g, struct game_move m) {
//     bitboard move = bb_set(0ULL, m.to);
//     bitboard moves = rook_pseudolegal_moves(g->occupied, m.color, m.from);
//     return (bool)(move & moves);
// }

// bool
// bishop_move_is_pseudolegal(struct game* g, struct game_move m) {
//     bitboard move = bb_set(0ULL, m.to);
//     bitboard moves = bishop_pseudolegal_moves(g->occupied, m.color, m.from);
//     return (bool)(move & moves);
// }

// bool
// queen_move_is_pseudolegal(struct game* g, struct game_move m) {
//     bitboard move = bb_set(0ULL, m.to);
//     bitboard moves = queen_pseudolegal_moves(g->occupied, m.color, m.from);
//     return (bool)(move & moves);
// }

// static bool
// game_move_is_pseudolegal(struct game* g, struct game_move m) {
//     if (m.color != g->turn) {
//         return false;
//     }
//     switch (m.piece) {
//         case PT_INVALID: return false;
//         case PT_PAWN: return pawn_move_is_pseudolegal(g, m);
//         case PT_KNIGHT: return knight_move_is_pseudolegal(g, m);
//         case PT_KING: return king_move_is_pseudolegal(g, m);
//         case PT_ROOK: return rook_move_is_pseudolegal(g, m);
//         case PT_BISHOP: return bishop_move_is_pseudolegal(g, m);
//         case PT_QUEEN: return queen_move_is_pseudolegal(g, m);
//         default: return false;
//     }
// }

void
print_game(struct game* g) {
    printf("nonoccupied:");
    print_bb(g->nonocc);
    printf("white:");
    print_bb(g->occ[PC_W]);
    printf("black:");
    print_bb(g->occ[PC_B]);
}

void
game_move_update_bbs(struct game* g, struct move m) {
    BB_MUT_MOVE(g->pcs[m.color][m.piece], m.from, m.to);
    BB_MUT_MOVE(g->occ[m.color], m.from, m.to);
    for (int pt = 0; pt < piecetype_cnt; pt++) {
        BB_MUT_POP(g->pcs[!m.color][pt], m.to);
        BB_MUT_POP(g->occ[!m.color], m.to);
        if (bb_set(0ULL, m.to) == g->enp_targets[!m.color]) {
            BB_MUT_POP(g->pcs[!m.color][pt],
                       m.color == PC_W ? m.to + 8 : m.to - 8);
        }
    }

    if (m.piece == K) {
        if (m.color == PC_W) {
            g->castling_rights[0] = false;
            g->castling_rights[1] = false;
        } else {
            g->castling_rights[2] = false;
            g->castling_rights[3] = false;
        }
    }
    if (m.piece == R) {
        if (m.from == 0) {
            g->castling_rights[3] = false;
        }
        if (m.from == 7) {
            g->castling_rights[2] = false;
        }
        if (m.from == 56) {
            g->castling_rights[1] = false;
        }
        if (m.from == 63) {
            g->castling_rights[0] = false;
        }
    }

    if (m.type == MT_CASTLE) {
        if (m.to == 62) {
            BB_MUT_MOVE(g->pcs[m.color][R], 63, 61);
            BB_MUT_MOVE(g->occ[m.color], 63, 61);
        }
        if (m.to == 58) {
            BB_MUT_MOVE(g->pcs[m.color][R], 56, 59);
            BB_MUT_MOVE(g->occ[m.color], 56, 59);
        }
        if (m.to == 6) {
            BB_MUT_MOVE(g->pcs[m.color][R], 7, 5);
            BB_MUT_MOVE(g->occ[m.color], 7, 5);
        }
        if (m.to == 2) {
            BB_MUT_MOVE(g->pcs[m.color][R], 0, 3);
            BB_MUT_MOVE(g->occ[m.color], 0, 3);
        }
    }

    g->nonocc = ~(g->occ[PC_B] | g->occ[PC_W]);

    g->enp_targets[m.color] = 0ULL;
    if (m.piece == P && m.color == PC_W && m.from / 8 == 6 && m.to / 8 == 4) {
        g->enp_targets[m.color] |= bb_set(0ULL, m.to + 8);
    }
    if (m.piece == P && m.color == PC_B && m.from / 8 == 1 && m.to / 8 == 3) {
        g->enp_targets[m.color] |= bb_set(0ULL, m.to - 8);
    }

    if (m.type == MT_PROMOTION) {
        enum piecetype piece;
        if (m.promotion == PT_INVALID) {
            char input;
            printf("Speficy the promotion [n/b/r/q]: ");
            scanf("%c", &input);
            switch (input) {
                case 'n': piece = N; break;
                case 'b': piece = B; break;
                case 'r': piece = R; break;
                case 'q': piece = Q; break;
                default: piece = Q;
            }
        } else {
            piece = m.promotion;
        }
        BB_MUT_POP(g->pcs[m.color][P], m.to);
        BB_MUT_SET(g->pcs[m.color][piece], m.to);
    }
}

bool
player_in_check(struct game* g, enum piececolor pc) {
    bitboard king = g->pcs[pc][K];
    bitboard reachable_by_enemy = reachable_squares(g, !pc);
    return (bool)(king & reachable_by_enemy);
}

bool
pleg_move_is_leg(struct game* g, struct move m) {
    struct game sim = *g;
    game_move_update_bbs(&sim, m);
    // printf("(((\nsim:\n");
    // print_game(&sim);
    // printf("))) => %c\n", !game_is_check(&sim, g->turn) ? 'Y' : 'N');
    return !player_in_check(&sim, g->turn);
}

struct movelist
leg_moves(struct game* g) {
    struct movelist ml = pleg_moves(g);
    struct movelist result = {{0}, 0};
    for (int i = 0; i < ml.count; i++) {
        if (pleg_move_is_leg(g, ml.moves[i])) {
            result.moves[result.count] = ml.moves[i];
            result.count++;
        }
    }
    if (g->castling_rights[0] && !(~g->nonocc & 0x0000000000000006ULL)
        && !(reachable_squares(g, !g->turn) & 0x000000000000000EULL)) {
        printf("castle0\n");
        result.moves[result.count] =
            (struct move){60, 62, PC_W, K, PT_INVALID, MT_CASTLE};
        result.count++;
    }
    if (g->castling_rights[1] && !(~g->nonocc & 0x0000000000000070ULL)
        && !(reachable_squares(g, !g->turn) & 0x0000000000000038ULL)) {
        printf("castle1\n");
        result.moves[result.count] =
            (struct move){60, 58, PC_W, K, PT_INVALID, MT_CASTLE};
        result.count++;
    }
    if (g->castling_rights[2] && !(~g->nonocc & 0x0600000000000000ULL)
        && !(reachable_squares(g, !g->turn) & 0x0E00000000000000ULL)) {
        printf("castle2\n");
        result.moves[result.count] =
            (struct move){4, 6, PC_B, K, PT_INVALID, MT_CASTLE};
        result.count++;
    }
    if (g->castling_rights[3] && !(~g->nonocc & 0x7000000000000000ULL)
        && !(reachable_squares(g, !g->turn) & 0x3800000000000000ULL)) {
        printf("castle3\n");
        result.moves[result.count] =
            (struct move){4, 2, PC_B, K, PT_INVALID, MT_CASTLE};
        result.count++;
    }
    return result;
}

bool
move_is_leg(struct game* g, struct move m) {
    // if (!game_move_is_pseudolegal(g, m)) {
    //     return false;
    // }
    struct movelist ml = leg_moves(g);
    for (int i = 0; i < ml.count; i++) {
        if (cmp_move(ml.moves[i], m)) {
            return true;
        }
    }
    return false;
}

bool
game_is_checkmate(struct game* g) {
    if (!player_in_check(g, g->turn)) {
        return false;
    }
    struct movelist ml = pleg_moves(g);
    for (int i = 0; i < ml.count; i++) {
        struct game sim = *g;
        game_move_update_bbs(&sim, ml.moves[i]);
        if (!player_in_check(&sim, g->turn)) {
            return false;
        }
    }
    return true;
}

int
game_move(struct game* g, struct move m) {
    if (!g->active) {
        return -1;
    }
    if (!move_is_leg(g, m)) {
        return -1;
    }
    game_move_update_bbs(g, m);
    g->turn = !g->turn;
    if (game_is_checkmate(g)) {
        g->active = false;
        return 0;
    }
    return 0;
}

int
ui_game_move(struct game* g, struct move m) {
    print_move(m);
    if (!g->active) {
        printf("Game isn't active");
        return -1;
    }
    if (!move_is_leg(g, m)) {
        printf("Ilegal move");
        return -1;
    }
    game_move_update_bbs(g, m);
    g->turn = !g->turn;
    if (game_is_checkmate(g)) {
        printf("Checkmate");
        g->active = false;
        return 0;
    }
    if (player_in_check(g, g->turn)) {
        printf("Check ");
    }
    return 0;
}
