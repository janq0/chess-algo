#ifndef CHESS_H
#define CHESS_H

#include "bitboard.h"
#include "defs.h"

/* Represents a chess position at a given point in time */
struct game {
    bitboard pcs[piececolor_cnt]
                [piecetype_cnt];  /* Squares with a given color and piece */
    bitboard occ[piececolor_cnt]; /* Squares occupied by a given color */
    bitboard nonocc;              /* Squares that aren't occupied */
    bitboard enp_targets[piececolor_cnt]; /* En passant target squares */
    bool castling_rights[4];              /* */
    enum piececolor turn;                 /* Color that moves next */
    bool active;                          /* Further moves can be made */
    bool check;
};

/* Return the starting position. */
struct game init_game(void);

/* 
 * Returns an infered move based on the information in the game struct. If
 * there doesn't exist any piece at from, then the color field will be -1.
 */
struct move infered_game_move(struct game* game, int from, int to);

/* Apply move m to game g. */
int game_move(struct game* g, struct move m);

int ui_game_move(struct game* g, struct move m);

void print_move(struct move m);

// bitboard R_pleg_targets(bitboard occ[2], enum piececolor pc, int sqr);
// bitboard B_pleg_targets(bitboard occ[2], enum piececolor pc, int sqr);
// bitboard Q_pleg_targets(bitboard occ[2], enum piececolor pc, int sqr);

/* Generates all legal moves. */
struct movelist leg_moves(struct game* g);
struct movelist pleg_moves(struct game* g);
bitboard reachable_squares(struct game* g, enum piececolor color);

#endif // CHESS_H