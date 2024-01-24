#ifndef BITBOARD_H
#define BITBOARD_H

#include <stdbool.h>
#include "defs.h"

#define BB_MUT_MOVE(BB, FROM, TO) (BB = bb_move(BB, FROM, TO))
#define BB_MUT_POP(BB, INDEX)     (BB = bb_pop(BB, INDEX))
#define BB_MUT_SET(BB, INDEX)     (BB = bb_set(BB, INDEX))

/* Bitboards should represent the squares left to right, top to bottom. */
typedef unsigned long long bitboard;

struct magic_entry {
    bitboard mask;
    bitboard multiplier;
    char shift;
};

extern const bitboard a8_bb, rank8_bb, rank7_bb, rank5_bb, rank4_bb, rank2_bb,
    rank1_bb, not_fileA_bb, not_fileH_bb;

extern bitboard N_pattern[64], K_pattern[64], P_atk_pattern[piececolor_cnt][64],
    P_push_pattern[piececolor_cnt], file_bbs[8];

extern bitboard piece_moves_bb[piececolor_cnt][piecetype_cnt][64];
extern bitboard pawn_capture_move[];

/* Prints bb in a nice format to stdout for debugging. */
void print_bb(bitboard bb);

/* Returns a truthy bitboard if the bit at index is 1. */
bitboard bb_get(bitboard bb, int index);

/* Returns the bitboard with the bit at index set to 1. */
bitboard bb_set(bitboard bb, int index);

/* Returns the bitboard with the bit at index set to 0. */
bitboard bb_pop(bitboard bb, int index);

/* Returns the bitboard with chained bb_pop and bb_set */
bitboard bb_move(bitboard bb, int from, int to);

int square_index(char coords[2]);

/**
 * "Using de Bruijn Sequences to Index a 1 in a Computer Word"
 * @param bb bitboard to scan
 * @precondition bb != 0
 * @return index (0..63) of least significant one bit
 */
int bb_ls1b(bitboard bb);

void gen_bbs();

#endif // BITBOARD_H