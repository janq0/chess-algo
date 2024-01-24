#ifndef SEARCH_H
#define SEARCH_H

#include "chess.h"
#include "defs.h"

int eval(struct game* g);
struct move best_move(struct game* g);

#endif // SEARCH_H