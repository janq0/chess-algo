/*
 * This module defines common constants and enumerations which are useful in
 * multiple modules. This approach
 * - gets rid of circular dependencies that arise when two or more modules
 *   need to import each other's constants or enumerations and
 * - keeps the enumeration values synced between all modules who use them.
 */

#ifndef DEFS_H
#define DEFS_H
#define MAX_MOVE_COUNT 300

enum piecetype { PT_INVALID = -1, K = 0, Q, R, B, N, P, piecetype_cnt };

enum piececolor { PC_INVALID = -1, PC_W = 0, PC_B = 1, piececolor_cnt };

enum movetype { MT_QUIET = 0, MT_ATTACK, MT_CASTLE, MT_PROMOTION };

struct move {
    int from, to;
    enum piececolor color;
    enum piecetype piece;
    enum piecetype promotion;
    enum movetype type;
};

struct movelist {
    struct move moves[MAX_MOVE_COUNT];
    int count;
};

#endif // DEFS_H