
#include <stdlib.h>

#include "../include/str.h"
#include "../include/pair.h"

Pair* Pair_new(char const* val, Pair* car, Pair* cdr) {

	Pair* pair = malloc(sizeof(Pair));

	if (!pair) return NULL;

	pair->val = val;
	pair->car = car;
	pair->cdr = cdr;

	pair->dyn = false;

	return pair;
}

Pair* Pair_dyn(char const* val, Pair* car, Pair* cdr) {

	Pair* pair = malloc(sizeof(Pair));

	if (!pair) return NULL;

	pair->val = strdup(val);
	pair->car = car;
	pair->cdr = cdr;

	pair->dyn = true;

	return pair;
}

void Pair_free(Pair* pair) {

	if (!pair) return;

	if (pair->dyn) {
		free((void*) pair->val);
	}
	
	if (pair->car) free(pair->car);
	if (pair->cdr) free(pair->cdr);

	free(pair);
}

Pair* Pair_last(Pair* pair) {

	Pair* prev;

	while (pair) {
		prev = pair;
		pair = pair->cdr;
	}

	return prev;
}