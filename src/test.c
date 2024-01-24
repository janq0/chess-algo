#include <stdio.h>
#include "bitboard.h"
#include "chess.h"
#include "defs.h"

void
test_bb(bitboard bb, bitboard expected) {
    if (bb != expected) {
        print_bb(bb);
    } else {
        printf(".");
    }
}

int
main(void) {
    gen_bbs();

    test_bb(P_atk_pattern[PC_W][0], 0x0000000000000000ULL);
    test_bb(P_atk_pattern[PC_W][1], 0x0000000000000000ULL);
    test_bb(P_atk_pattern[PC_W][7], 0x0000000000000000ULL);
    test_bb(P_atk_pattern[PC_W][8], 0x4000000000000000ULL);
    test_bb(P_atk_pattern[PC_W][9], 0xA000000000000000ULL);
    test_bb(P_atk_pattern[PC_W][10], 0x5000000000000000ULL);
    test_bb(P_atk_pattern[PC_W][14], 0x0500000000000000ULL);
    test_bb(P_atk_pattern[PC_W][15], 0x0200000000000000ULL);
    test_bb(P_atk_pattern[PC_W][36], 0x0000001400000000ULL);
    test_bb(P_atk_pattern[PC_W][56], 0x0000000000004000ULL);
    test_bb(P_atk_pattern[PC_W][63], 0x0000000000000200ULL);

    test_bb(N_pattern[0], 0x0020400000000000ULL);
    test_bb(N_pattern[1], 0x10A00000000000ULL);
    test_bb(N_pattern[2], 0x88500000000000ULL);
    test_bb(N_pattern[5], 0x110A0000000000ULL);
    test_bb(N_pattern[6], 0x8050000000000ULL);
    test_bb(N_pattern[7], 0x4020000000000ULL);
    test_bb(N_pattern[8], 0x2000204000000000ULL);
    test_bb(N_pattern[9], 0x100010A000000000ULL);
    test_bb(N_pattern[27], 0x28440044280000ULL);
    test_bb(N_pattern[50], 0x50880088ULL);
    test_bb(N_pattern[61], 0xA1100ULL);

    test_bb(K_pattern[0], 0x40C0000000000000ULL);
    test_bb(K_pattern[1], 0xA0E0000000000000ULL);
    test_bb(K_pattern[7], 0x203000000000000ULL);
    test_bb(K_pattern[24], 0xC040C0000000ULL);
    test_bb(K_pattern[26], 0x705070000000ULL);
    test_bb(K_pattern[31], 0x30203000000ULL);
    test_bb(K_pattern[56], 0xC040ULL);
    test_bb(K_pattern[59], 0x3828ULL);
    test_bb(K_pattern[63], 0x302ULL);

    bitboard occ[2] = {0ULL, 0ULL};
    test_bb(R_pleg_targets(occ, PC_W, 35), 0x10101010EF101010ULL);
    test_bb(R_pleg_targets(occ, PC_W, 0), 0x7F80808080808080ULL);
    test_bb(R_pleg_targets(occ, PC_W, 7), 0xFE01010101010101ULL);
    test_bb(R_pleg_targets(occ, PC_W, 56), 0x808080808080807FULL);
    test_bb(R_pleg_targets(occ, PC_W, 63), 0x1010101010101FEULL);
    occ[0] = bb_set(occ[0], 3);
    occ[0] = bb_set(occ[0], 33);
    occ[0] = bb_set(occ[0], 36);
    occ[0] = bb_set(occ[0], 51);
    test_bb(R_pleg_targets(occ, PC_W, 35), 0x10101020100000ULL);

    occ[0] = 0ULL;
    occ[1] = 0ULL;
    test_bb(B_pleg_targets(occ, PC_W, 35), 0x182442800284482ULL);
    test_bb(B_pleg_targets(occ, PC_W, 0), 0x40201008040201ULL);
    test_bb(B_pleg_targets(occ, PC_W, 7), 0x2040810204080ULL);
    test_bb(B_pleg_targets(occ, PC_W, 56), 0x102040810204000ULL);
    test_bb(B_pleg_targets(occ, PC_W, 63), 0x8040201008040200ULL);
    occ[0] = bb_set(occ[0], 8);
    occ[0] = bb_set(occ[0], 21);
    occ[1] = bb_set(occ[1], 56);
    occ[1] = bb_set(occ[1], 44);
    test_bb(B_pleg_targets(occ, PC_W, 35), 0x402800284080ULL);
    test_bb(Q_pleg_targets(occ, PC_W, 35), 0x10105038EF385090ULL);
    test_bb(Q_pleg_targets(occ, PC_B, 35), 0x10905438EF305010ULL);

    struct game g = init_game();
    test_bb(reachable_squares(&g), 0xFFFF0000ULL);
}
