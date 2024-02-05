#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "bitboard.h"

int RBits[64] = {12, 11, 11, 11, 11, 11, 11, 12, 11, 10, 10, 10, 10,
                 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10,
                 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10,
                 11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10,
                 10, 10, 10, 11, 12, 11, 11, 11, 11, 11, 11, 12};

int BBits[64] = {6, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 5, 5,
                 5, 5, 7, 7, 7, 7, 5, 5, 5, 5, 7, 9, 9, 7, 5, 5,
                 5, 5, 7, 9, 9, 7, 5, 5, 5, 5, 7, 7, 7, 7, 5, 5,
                 5, 5, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 6};

bitboard
random_bb() {
    bitboard bb1 = (bitboard)random() & 0xFFFF;
    bitboard bb2 = (bitboard)random() & 0xFFFF;
    bitboard bb3 = (bitboard)random() & 0xFFFF;
    bitboard bb4 = (bitboard)random() & 0xFFFF;
    return bb1 | bb2 << 16 | bb3 << 32 | bb4 << 48;
}

bitboard
random_sparse_bb() {
    bitboard bb1 = random_bb();
    bitboard bb2 = random_bb();
    bitboard bb3 = random_bb();
    return bb1 & bb2 & bb3;
}

int
magic_hash_(bitboard blockers, bitboard magic, int shift) {
    return magic * blockers >> (64 - shift);
}

int R_shift[64] = {12, 11, 11, 11, 11, 11, 11, 12, 11, 10, 10, 10, 10,
                   10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10,
                   10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10,
                   11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10,
                   10, 10, 10, 11, 12, 11, 11, 11, 11, 11, 11, 12};

int B_shift[64] = {6, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 5, 5,
                   5, 5, 7, 7, 7, 7, 5, 5, 5, 5, 7, 9, 9, 7, 5, 5,
                   5, 5, 7, 9, 9, 7, 5, 5, 5, 5, 7, 7, 7, 7, 5, 5,
                   5, 5, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 6};

void
test_m(int sq, int sh, bitboard magic, bitboard mask) {
    bitboard visited[4096] = {0ULL};
    printf("testing sq=%d magic=%llx:\n", sq, magic);
    bitboard blockers = 0ULL;
    do {
        int hash = magic_hash_(blockers, magic, sh);
        bitboard atk = B_targets_slow(sq, blockers);
        if (visited[hash] == 0ULL) {
            visited[hash] = atk;
        }
        if (visited[hash] != atk) {
            printf("FAIL!\n");
            return;
        }
        mask = (blockers - mask) & mask;
    } while (blockers);
}

bitboard
find_B_magic(int sq) {
    bitboard blockers[8200], targets[8200], used[8200];
    bitboard mask = B_magic_mask(sq);
    bool failed;
    int n = bb_count_1s(mask);

    blockers[0] = 0ULL;
    targets[0] = B_targets_slow(sq, 0ULL);
    int i = 0;
    do {
        i++;
        blockers[i] = (blockers[i - 1] - mask) & mask;
        targets[i] = B_targets_slow(sq, blockers[i]);
    } while (blockers[i]);
    for (int k = 0; k < 1000000000; k++) {
        bitboard magic = random_sparse_bb();
        if (bb_count_1s(mask * magic & 0xFF00000000000000ULL) < 6) {
            continue;
        }
        for (i = 0; i < 4096; i++) {
            used[i] = 0ULL;
        }
        for (i = 0, failed = false; !failed && i < 1 << n; i++) {
            int hash = magic_hash_(blockers[i], magic, B_shift[sq]);
            if (used[hash] == 0ULL) {
                used[hash] = targets[i];
            } else if (used[hash] != targets[i]) {
                failed = true;
            }
        }
        if (!failed) {
            // test_m(sq, B_shift[sq], magic, mask);
            return magic;
        }
    }
    printf("FAILED\n");
    return 0ULL;
}

bitboard
find_R_magic(int sq) {
    bitboard blockers[8200], targets[8200], used[8200];
    bitboard mask = R_magic_mask(sq);
    bool failed;
    int n = bb_count_1s(mask);

    blockers[0] = 0ULL;
    targets[0] = R_targets_slow(sq, 0ULL);
    int i = 0;
    do {
        i++;
        blockers[i] = (blockers[i - 1] - mask) & mask;
        targets[i] = R_targets_slow(sq, blockers[i]);
    } while (blockers[i]);
    for (int k = 0; k < 10000000; k++) {
        bitboard magic = random_sparse_bb();
        if (bb_count_1s(mask * magic & 0xFF00000000000000ULL) < 6) {
            continue;
        }
        for (i = 0; i < 4096; i++) {
            used[i] = 0ULL;
        }
        for (i = 0, failed = false; !failed && i < 1 << n; i++) {
            int hash = magic_hash_(blockers[i], magic, R_shift[sq]);
            if (used[hash] == 0ULL) {
                used[hash] = targets[i];
            } else if (used[hash] != targets[i]) {
                failed = true;
                printf("");
            };
        }
        if (!failed) {
            // test_m(sq, R_shift[sq], magic, mask);
            return magic;
        }
    }
    printf("FAILED\n");
    return 0ULL;
}

int
main() {
    //srand(time(NULL));
    printf("const struct magic_entry B_magics[64] = {\n");
    for (int sq = 0; sq < 64; sq++) {
        printf("    {0x%llxULL, 0x%llxULL, %d},\n", B_magic_mask(sq), find_B_magic(sq), B_shift[sq]);
    }
    printf("const struct magic_entry R_magics[64] = {\n");
    for (int sq = 0; sq < 64; sq++) {
        printf("    {0x%llxULL, 0x%llxULL, %d},\n", R_magic_mask(sq), find_R_magic(sq), R_shift[sq]);
    }
    printf("};\n");
}
