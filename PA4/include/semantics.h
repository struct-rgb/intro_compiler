
#ifndef SEMANTICS_H
#define SEMANTICS_H

#include "pair.h"

typedef enum Semantic {
	/* 0 */ Semantic_OK,
	/* 1 */ Semantic_TYPE_ERROR,
	/* 2 */ Semantic_REDECLARATION,
	/* 3 */ Semantic_RETURN_ERROR,
	/* 4 */ Semantic_UNDECLARED_SYMBOL,
	/* 5 */ Semantic_VOID_VAR,
	/* 6 */ Semantic_NO_FINAL_VOID_MAIN_VOID,
	/* 7 */ Semantic_INTERNAL_ERROR,
	/* 8 */ Semantic_INVALID_STATE,
	/* 9 */ Semantic_ARITY_MISMATCH,
	/* A */ Semantic_BAD_LITERAL_VALUE
} Semantic;

Semantic check_semantics(Pair* ast);

#endif