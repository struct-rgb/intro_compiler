
#ifndef PAIR_H
#define PAIR_H

#include <stdbool.h>

typedef struct Pair {
	char const*  val;
	struct Pair* car;
	struct Pair* cdr;
	bool         dyn;
} Pair;

Pair* Pair_new(char const* val, Pair* car, Pair* cdr);

Pair* Pair_dyn(char const* val, Pair* car, Pair* cdr);

void Pair_free(Pair* pair);

Pair* Pair_last(Pair* pair);

#endif