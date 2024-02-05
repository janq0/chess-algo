#include "bitboard.h"
#include <assert.h>
#include <stdio.h>
#include "defs.h"
#include "magicvalues.c"

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
         B_pattern[64][4096] = {0ULL}, R_pattern[64][4096],
         P_push_pattern[piececolor_cnt] = {0ULL}, file_bbs[8] = {0ULL};

struct {
    const int index[64]; /* Maps the subsequence it's index in the sequence */
    const bitboard seq;  /* An order 6 binary De Bruijn sequence */
} bb_debruijn = {
    {0,  1,  48, 2,  57, 49, 28, 3,  61, 58, 50, 42, 38, 29, 17, 4,
     62, 55, 59, 36, 53, 51, 43, 22, 45, 39, 33, 30, 24, 18, 12, 5,
     63, 47, 56, 27, 60, 41, 37, 16, 54, 35, 52, 21, 44, 32, 23, 11,
     46, 26, 40, 15, 34, 20, 31, 10, 25, 14, 19, 9,  13, 8,  7,  6},
    0x03f79d71b4cb0a89ULL,
};
struct magic_entry B_magics[64] = {
    {0x40201008040200ULL, 0x420c80100408202ULL, 6},
    {0x20100804020000ULL, 0x1063090051100ULL, 5},
    {0x50080402000000ULL, 0x32a0201061110104ULL, 5},
    {0x28440200000000ULL, 0x501025140082210ULL, 5},
    {0x14224000000000ULL, 0x820040120a801ULL, 5},
    {0xa102040000000ULL, 0x2014820084012100ULL, 5},
    {0x4081020400000ULL, 0x81008201012100ULL, 5},
    {0x2040810204000ULL, 0x2402208124020ULL, 6},
    {0x402010080400ULL, 0x402080809104010ULL, 5},
    {0x201008040200ULL, 0x42c0100111310000ULL, 5},
    {0x500804020000ULL, 0x104002040440b0ULL, 5},
    {0x284402000000ULL, 0x31020220250ULL, 5},
    {0x142240000000ULL, 0x20020880c00ULL, 5},
    {0xa1020400000ULL, 0x8000d20222090000ULL, 5},
    {0x40810204000ULL, 0x308c04202000ULL, 5},
    {0x20408102000ULL, 0x2020282400042ULL, 5},
    {0x40004020100800ULL, 0x1044010673011201ULL, 5},
    {0x20002010080400ULL, 0x200484004a4092ULL, 5},
    {0x50005008040200ULL, 0x40010208a2000100ULL, 7},
    {0x28002844020000ULL, 0x28088902400401ULL, 7},
    {0x14001422400000ULL, 0x2000014200804800ULL, 7},
    {0xa000a10204000ULL, 0xa803040044000840ULL, 7},
    {0x4000408102000ULL, 0x802010121020801ULL, 5},
    {0x2000204081000ULL, 0x100903050200800ULL, 5},
    {0x20400040201000ULL, 0x12600800a0044ULL, 5},
    {0x10200020100800ULL, 0x4044612400020080ULL, 5},
    {0x8500050080400ULL, 0x910004080831000ULL, 7},
    {0x44280028440200ULL, 0x2430008200002200ULL, 9},
    {0x22140014224000ULL, 0x100200800650104ULL, 9},
    {0x100a000a102000ULL, 0x4002080040500ULL, 7},
    {0x8040004081000ULL, 0x42101105020200ULL, 5},
    {0x4020002040800ULL, 0x801480803401000ULL, 5},
    {0x10204000402000ULL, 0x490840840100ULL, 5},
    {0x8102000201000ULL, 0x2408002009000ULL, 5},
    {0x4085000500800ULL, 0x10004102081208ULL, 7},
    {0x2442800284400ULL, 0x100102500400cULL, 9},
    {0x40221400142200ULL, 0x8440040018410120ULL, 9},
    {0x20100a000a1000ULL, 0x1000410008160400ULL, 7},
    {0x10080400040800ULL, 0xa801200088083110ULL, 5},
    {0x8040200020400ULL, 0x10400808024400ULL, 5},
    {0x8102040004000ULL, 0x80020a088201ULL, 5},
    {0x4081020002000ULL, 0x601000261105000ULL, 5},
    {0x2040850005000ULL, 0x4063020a00610c00ULL, 7},
    {0x24428002800ULL, 0x8809000820082000ULL, 7},
    {0x402214001400ULL, 0x8000a20801010ULL, 7},
    {0x4020100a000a00ULL, 0x10000200204100ULL, 7},
    {0x20100804000400ULL, 0x281c082001120614ULL, 5},
    {0x10080402000200ULL, 0x8040011010024089ULL, 5},
    {0x4081020400000ULL, 0x208000828c012000ULL, 5},
    {0x2040810200000ULL, 0xc0401880831ULL, 5},
    {0x20408500000ULL, 0x121110080020ULL, 5},
    {0x244280000ULL, 0x408040308064302ULL, 5},
    {0x4022140000ULL, 0x285040400880000ULL, 5},
    {0x4020100a0000ULL, 0x287801022011ULL, 5},
    {0x40201008040000ULL, 0x2c00200808a10240ULL, 5},
    {0x20100804020000ULL, 0x80110308210400ULL, 5},
    {0x2040810204000ULL, 0x100120082201000ULL, 6},
    {0x20408102000ULL, 0x200940402400220ULL, 5},
    {0x204085000ULL, 0x8021018840800840ULL, 5},
    {0x2442800ULL, 0x8028484001009010ULL, 5},
    {0x40221400ULL, 0x9a1104300040000ULL, 5},
    {0x4020100a00ULL, 0x1008008400850000ULL, 5},
    {0x402010080400ULL, 0x2004104401002088ULL, 5},
    {0x40201008040200ULL, 0x48011034104480ULL, 6},
};

struct magic_entry R_magics[64] = {
    {0x7e80808080808000ULL, 0x800008044002102ULL, 12},
    {0x3e40404040404000ULL, 0x1000a1281204ULL, 11},
    {0x5e20202020202000ULL, 0x8531000208040001ULL, 11},
    {0x6e10101010101000ULL, 0x9001001042080005ULL, 11},
    {0x7608080808080800ULL, 0x8a01000410002009ULL, 11},
    {0x7a04040404040400ULL, 0x400200040081101ULL, 11},
    {0x7c02020202020200ULL, 0x190080400421ULL, 11},
    {0x7e01010101010100ULL, 0x111004221908009ULL, 12},
    {0x7e808080808000ULL, 0x800041002080ULL, 11},
    {0x3e404040404000ULL, 0x20128900400ULL, 10},
    {0x5e202020202000ULL, 0x44004002010040ULL, 10},
    {0x6e101010101000ULL, 0x40008008080ULL, 10},
    {0x76080808080800ULL, 0x420008201200ULL, 10},
    {0x7a040404040400ULL, 0x4012004080102200ULL, 10},
    {0x7c020202020200ULL, 0x4c0200040100040ULL, 10},
    {0x7e010101010100ULL, 0x400080002280ULL, 11},
    {0x807e8080808000ULL, 0x24008100420004ULL, 11},
    {0x403e4040404000ULL, 0x126010208440010ULL, 10},
    {0x205e2020202000ULL, 0x100040002008080ULL, 10},
    {0x106e1010101000ULL, 0xc1001008010004ULL, 10},
    {0x8760808080800ULL, 0x1020100008008080ULL, 10},
    {0x47a0404040400ULL, 0x130004020010100ULL, 10},
    {0x27c0202020200ULL, 0x1120005000c002ULL, 10},
    {0x17e0101010100ULL, 0x8050802040008004ULL, 11},
    {0x80807e80808000ULL, 0x411084402000091ULL, 11},
    {0x40403e40404000ULL, 0x1018800100800200ULL, 10},
    {0x20205e20202000ULL, 0x2000204008010410ULL, 10},
    {0x10106e10101000ULL, 0x480005001100ULL, 10},
    {0x8087608080800ULL, 0xc430001080800800ULL, 10},
    {0x4047a04040400ULL, 0x5c04108202002040ULL, 10},
    {0x2027c02020200ULL, 0x408a054382002100ULL, 10},
    {0x1017e01010100ULL, 0x7080004000402000ULL, 11},
    {0x8080807e808000ULL, 0x4008054200032194ULL, 11},
    {0x4040403e404000ULL, 0x4888084400011002ULL, 10},
    {0x2020205e202000ULL, 0x100040080020080ULL, 10},
    {0x1010106e101000ULL, 0x81001100080084ULL, 10},
    {0x8080876080800ULL, 0x2030080080100080ULL, 10},
    {0x404047a040400ULL, 0x8010080020040022ULL, 10},
    {0x202027c020200ULL, 0x4040200880400080ULL, 10},
    {0x101017e010100ULL, 0x8040400080008032ULL, 11},
    {0x808080807e8000ULL, 0x2024020000b40541ULL, 11},
    {0x404040403e4000ULL, 0x6004080400100ULL, 10},
    {0x202020205e2000ULL, 0x809010002080400ULL, 10},
    {0x101010106e1000ULL, 0xa14008004080080ULL, 10},
    {0x8080808760800ULL, 0x408010100201000ULL, 10},
    {0x40404047a0400ULL, 0x7011460020118200ULL, 10},
    {0x20202027c0200ULL, 0x8a0004000500025ULL, 10},
    {0x10101017e0100ULL, 0x1180014000402000ULL, 11},
    {0x80808080807e00ULL, 0x102000c02045181ULL, 11},
    {0x40404040403e00ULL, 0x602a004802000104ULL, 10},
    {0x20202020205e00ULL, 0x8402000410080200ULL, 10},
    {0x10101010106e00ULL, 0x8800800800040080ULL, 10},
    {0x8080808087600ULL, 0x8001001000200902ULL, 10},
    {0x4040404047a00ULL, 0x108808020001000ULL, 10},
    {0x2020202027c00ULL, 0x2803002181400300ULL, 10},
    {0x1010101017e00ULL, 0x8003800080400020ULL, 11},
    {0x8080808080807eULL, 0x4300004022008100ULL, 12},
    {0x4040404040403eULL, 0x300110009d40200ULL, 11},
    {0x2020202020205eULL, 0x180020001800400ULL, 11},
    {0x1010101010106eULL, 0x8280040002800800ULL, 11},
    {0x8080808080876ULL, 0x4100041000210008ULL, 11},
    {0x404040404047aULL, 0x8180100080200108ULL, 11},
    {0x202020202027cULL, 0x440001000402000ULL, 11},
    {0x101010101017eULL, 0x80058055204000ULL, 12},
};

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
    assert(bb != 0);
    return 63 - bb_debruijn.index[((bb & -bb) * bb_debruijn.seq) >> 58];
}

int
bb_count_1s(bitboard bb) {
    int r;
    for (r = 0; bb; r++, bb &= bb - 1)
        ;
    return r;
}

bitboard
R_magic_mask(int sq) {
    bitboard result = 0ULL;
    int rk = sq / 8, fl = sq % 8, r, f;
    for (r = rk + 1; r <= 6; r++) {
        result |= bb_set(0ULL, fl + r * 8);
    }
    for (r = rk - 1; r >= 1; r--) {
        result |= bb_set(0ULL, fl + r * 8);
    }
    for (f = fl + 1; f <= 6; f++) {
        result |= bb_set(0ULL, f + rk * 8);
    }
    for (f = fl - 1; f >= 1; f--) {
        result |= bb_set(0ULL, f + rk * 8);
    }
    return result;
}

bitboard
B_magic_mask(int sq) {
    bitboard result = 0ULL;
    int rk = sq / 8, fl = sq % 8, r, f;
    for (r = rk + 1, f = fl + 1; r <= 6 && f <= 6; r++, f++) {
        result |= bb_set(0ULL, f + r * 8);
    }
    for (r = rk + 1, f = fl - 1; r <= 6 && f >= 1; r++, f--) {
        result |= bb_set(0ULL, f + r * 8);
    }
    for (r = rk - 1, f = fl + 1; r >= 1 && f <= 6; r--, f++) {
        result |= bb_set(0ULL, f + r * 8);
    }
    for (r = rk - 1, f = fl - 1; r >= 1 && f >= 1; r--, f--) {
        result |= bb_set(0ULL, f + r * 8);
    }
    return result;
}

bitboard
R_targets_slow(int sq, bitboard block) {
    bitboard result = 0ULL;
    int rk = sq / 8, fl = sq % 8, r, f;
    for (r = rk + 1; r <= 7; r++) {
        result |= bb_set(0ULL, fl + r * 8);
        if (block & bb_set(0ULL, fl + r * 8)) {
            break;
        }
    }
    for (r = rk - 1; r >= 0; r--) {
        result |= bb_set(0ULL, fl + r * 8);
        if (block & bb_set(0ULL, fl + r * 8)) {
            break;
        }
    }
    for (f = fl + 1; f <= 7; f++) {
        result |= bb_set(0ULL, f + rk * 8);
        if (block & bb_set(0ULL, f + rk * 8)) {
            break;
        }
    }
    for (f = fl - 1; f >= 0; f--) {
        result |= bb_set(0ULL, f + rk * 8);
        if (block & bb_set(0ULL, f + rk * 8)) {
            break;
        }
    }
    return result;
}

bitboard
B_targets_slow(int sq, bitboard block) {
    bitboard result = 0ULL;
    int rk = sq / 8, fl = sq % 8, r, f;
    for (r = rk + 1, f = fl + 1; r <= 7 && f <= 7; r++, f++) {
        result |= bb_set(0ULL, f + r * 8);
        if (block & bb_set(0ULL, f + r * 8)) {
            break;
        }
    }
    for (r = rk + 1, f = fl - 1; r <= 7 && f >= 0; r++, f--) {
        result |= bb_set(0ULL, f + r * 8);
        if (block & bb_set(0ULL, f + r * 8)) {
            break;
        }
    }
    for (r = rk - 1, f = fl + 1; r >= 0 && f <= 7; r--, f++) {
        result |= bb_set(0ULL, f + r * 8);
        if (block & bb_set(0ULL, f + r * 8)) {
            break;
        }
    }
    for (r = rk - 1, f = fl - 1; r >= 0 && f >= 0; r--, f--) {
        result |= bb_set(0ULL, f + r * 8);
        if (block & bb_set(0ULL, f + r * 8)) {
            break;
        }
    }
    return result;
}

int
B_magic_hash(int sq, bitboard blockers) {
    struct magic_entry e = B_magics[sq];
    return (e.multiplier * (e.mask & blockers)) >> (64 - e.shift);
}

int
R_magic_hash(int sq, bitboard blockers) {
    struct magic_entry e = R_magics[sq];
    return (e.multiplier * (e.mask & blockers)) >> (64 - e.shift);
}

void
gen_magic_lookups(int sq) {
    bitboard blocker_mask, blockers;
    int magic;

    blocker_mask = R_magic_mask(sq);
    blockers = 0ULL;
    do {
        magic = R_magic_hash(sq, blockers);
        R_pattern[sq][magic] = R_targets_slow(sq, blockers);
        blockers = (blockers - blocker_mask) & blocker_mask;
    } while (blockers);

    blocker_mask = B_magic_mask(sq);
    blockers = 0ULL;
    do {
        magic = B_magic_hash(sq, blockers);
        B_pattern[sq][magic] = B_targets_slow(sq, blockers);
        blockers = (blockers - blocker_mask) & blocker_mask;
    } while (blockers);
}

void
gen_bbs(void) {
    for (int col = 0; col < 8; col++) {
        for (int row = 0; row < 8; row++) {
            BB_MUT_SET(file_bbs[col], col + 8 * row);
        }
    }
    for (int i = 0; i < 64; i++) {
        gen_magic_lookups(i);

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
