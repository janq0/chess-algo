#include "bitboard.h"
#include <assert.h>
#include <stdio.h>
#include "defs.h"

const bitboard a8_bb = 0x8000000000000000ULL, rank8_bb = 0xFF00000000000000ULL,
               rank7_bb = 0x00FF000000000000ULL,
               rank5_bb = 0x000000FF00000000ULL,
               rank4_bb = 0x0000000FF000000ULL,
               rank2_bb = 0x000000000000FF00ULL,
               rank1_bb = 0x00000000000000FFULL,
               not_fileA_bb = 0x7F7F7F7F7F7F7F7FULL,
               not_fileH_bb = 0xFEFEFEFEFEFEFEFEULL,
               not_fileAB_bb = 0x3F3F3F3F3F3F3F3FULL,
               not_fileGH_bb = 0xFCFCFCFCFCFCFCFCULL,
               not_rank1_bb = 0xFFFFFFFFFFFFFF00ULL,
               not_rank8_bb = 0x00FFFFFFFFFFFFFFULL,
               not_rank12_bb = 0xFFFFFFFFFFFF0000ULL,
               not_rank78_bb = 0x0000FFFFFFFFFFFFULL;

bitboard N_pattern[64] = {0ULL}, K_pattern[64] = {0ULL},
         P_atk_pattern[piececolor_cnt][64] = {0ULL},
         P_push_pattern[piececolor_cnt] = {0ULL}, file_bbs[8] = {0ULL};

struct magic_entry rook_magics[64], bishop_magics[64];

void
print_bb(bitboard bb) {
    printf("(%llX)\n", bb);
    int index;
    for (int rank = 1; rank <= 8; rank++) {
        printf("%d", rank);
        for (int file = 1; file <= 8; file++) {
            index = (rank - 1) * 8 + (file - 1);
            printf(" %c", bb_get(bb, index) ? '#' : '.');
        }
        printf("\n");
    }
    printf("  a b c d e f g h\n\n");
}

inline bitboard
bb_get(bitboard bb, int index) {
    return bb & (a8_bb >> index);
}

inline bitboard
bb_set(bitboard bb, int index) {
    return bb | (a8_bb >> index);
}

inline bitboard
bb_pop(bitboard bb, int index) {
    return bb & ~(a8_bb >> index);
}

inline bitboard
bb_move(bitboard bb, int from, int to) {
    return bb_set(bb_pop(bb, from), to);
}

inline int
square_index(char coords[2]) {
    int file = coords[0] - 'a';
    int rank = 7 - (coords[1] - '1');
    return file + 8 * rank;
}

int
bb_ls1b(bitboard bb) {
    const int index64[64] = {0,  1,  48, 2,  57, 49, 28, 3,  61, 58, 50, 42, 38,
                             29, 17, 4,  62, 55, 59, 36, 53, 51, 43, 22, 45, 39,
                             33, 30, 24, 18, 12, 5,  63, 47, 56, 27, 60, 41, 37,
                             16, 54, 35, 52, 21, 44, 32, 23, 11, 46, 26, 40, 15,
                             34, 20, 31, 10, 25, 14, 19, 9,  13, 8,  7,  6};
    const bitboard debruijn64 = 0x03f79d71b4cb0a89ULL;
    assert(bb != 0);
    return 63 - index64[((bb & -bb) * debruijn64) >> 58];
}

void
gen_bbs(void) {
    for (int col = 0; col < 8; col++) {
        for (int row = 0; row < 8; row++) {
            BB_MUT_SET(file_bbs[col], col + 8 * row);
        }
        print_bb(file_bbs[col]);
    }
    for (int i = 0; i < 64; i++) {
        bitboard piece_bb = bb_set(0ULL, i);
        // Pawns
        if (piece_bb & not_fileA_bb) {
            if (piece_bb & not_rank8_bb) {
                P_atk_pattern[PC_W][i] |= piece_bb << 9;
            }
            if (piece_bb & not_rank1_bb) {
                P_atk_pattern[PC_B][i] |= piece_bb >> 7;
            }
        }
        if (piece_bb & not_fileH_bb) {
            if (piece_bb & not_rank8_bb) {
                P_atk_pattern[PC_W][i] |= piece_bb << 7;
            }
            if (piece_bb & not_rank1_bb) {
                P_atk_pattern[PC_B][i] |= piece_bb >> 9;
            }
        }
        // Knights
        if (piece_bb & not_fileAB_bb) {
            if (piece_bb & not_rank8_bb) {
                N_pattern[i] |= piece_bb << 10;
            }
            if (piece_bb & not_rank1_bb) {
                N_pattern[i] |= piece_bb >> 6;
            }
        }
        if (piece_bb & not_fileGH_bb) {
            if (piece_bb & not_rank8_bb) {
                N_pattern[i] |= piece_bb << 6;
            }
            if (piece_bb & not_rank1_bb) {
                N_pattern[i] |= piece_bb >> 10;
            }
        }
        if (piece_bb & not_fileA_bb) {
            if (piece_bb & not_rank78_bb) {
                N_pattern[i] |= piece_bb << 17;
            }
            if (piece_bb & not_rank12_bb) {
                N_pattern[i] |= piece_bb >> 15;
            }
        }
        if (piece_bb & not_fileH_bb) {
            if (piece_bb & not_rank78_bb) {
                N_pattern[i] |= piece_bb << 15;
            }
            if (piece_bb & not_rank12_bb) {
                N_pattern[i] |= piece_bb >> 17;
            }
        }
        // King
        if (piece_bb & not_rank8_bb) {
            K_pattern[i] |= piece_bb << 8;
            if (piece_bb & not_fileA_bb) {
                K_pattern[i] |= piece_bb << 9;
            }
            if (piece_bb & not_fileH_bb) {
                K_pattern[i] |= piece_bb << 7;
            }
        }
        if (piece_bb & not_rank1_bb) {
            K_pattern[i] |= piece_bb >> 8;
            if (piece_bb & not_fileA_bb) {
                K_pattern[i] |= piece_bb >> 7;
            }
            if (piece_bb & not_fileH_bb) {
                K_pattern[i] |= piece_bb >> 9;
            }
        }
        if (piece_bb & not_fileA_bb) {
            K_pattern[i] |= piece_bb << 1;
        }
        if (piece_bb & not_fileH_bb) {
            K_pattern[i] |= piece_bb >> 1;
        }
    }
}