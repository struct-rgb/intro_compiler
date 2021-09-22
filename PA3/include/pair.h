
#ifndef PAIR_H
#define PAIR_H

#include <stdbool.h>

typedef enum ASType {
	ASType_INT,
	ASType_VOID,
	ASType_ID,
	ASType_NUM,
	ASType_LE,
	ASType_LT,
	ASType_GT,
	ASType_GE,
	ASType_EQ,
	ASType_NE,
	ASType_ADD,
	ASType_SUB,
	ASType_MUL,
	ASType_DIV,
	ASType_ARGS,
	ASType_CALL,
	ASType_VAR,
	ASType_SET,
	ASType_EMPTY_STMT,
	ASType_RETURN_STMT,
	ASType_ITERATION_STMT,
	ASType_SELECTION_STMT,
	ASType_ARRAY,
	ASType_PARAM,
	ASType_PARAMS,
	ASType_COMPOUND_STMT,
	ASType_FUN_DECLARATION,
	ASType_VAR_DECLARATION,
	ASType_PROGRAM,
	ASType_NONE
} ASType;

typedef struct Pair {
	ASType       val;
	int          num;
	struct Pair* car;
	struct Pair* cdr;
	char const*  dyn;
} Pair;

Pair* Pair_new(ASType val, Pair* car, Pair* cdr);

Pair* Pair_dyn(ASType val, char const* dyn, Pair* car, Pair* cdr);

void Pair_free(Pair* pair);

Pair* Pair_last(Pair* pair);

#endif