
#include <stdlib.h>

#include "../include/str.h"
#include "../include/pair.h"

Pair* Pair_new(ASType val, Pair* car, Pair* cdr) {

	Pair* pair = malloc(sizeof(Pair));

	if (!pair) return NULL;

	pair->val = val;
	pair->num = 0;
	pair->car = car;
	pair->cdr = cdr;

	pair->dyn = NULL;

	return pair;
}

Pair* Pair_dyn(ASType val, char const* dyn, Pair* car, Pair* cdr) {

	Pair* pair = malloc(sizeof(Pair));

	if (!pair) return NULL;

	pair->val = val;
	pair->num = 0;
	pair->car = car;
	pair->cdr = cdr;

	pair->dyn = strdup(dyn);

	return pair;
}

void Pair_free(Pair* pair) {

	if (!pair) return;

	if (pair->dyn) {
		free((void*) pair->dyn);
	}
	
	if (pair->car) free(pair->car);
	if (pair->cdr) free(pair->cdr);

	free(pair);
}

Pair* Pair_last(Pair* pair) {

	Pair* prev = NULL;

	while (pair) {
		prev = pair;
		pair = pair->cdr;
	}

	return prev;
}