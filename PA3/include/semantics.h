
#ifndef SEMANTICS_H
#define SEMANTICS_H

#include "pair.h"

typedef enum Semantic {
	Semantic_OK,
	Semantic_TYPE_ERROR,
	Semantic_REDECLARATION,
	Semantic_RETURN_ERROR,
	Semantic_UNDECLARED_SYMBOL,
	Semantic_VOID_VAR,
	Semantic_NO_FINAL_VOID_MAIN_VOID,
	Semantic_INTERNAL_ERROR,
	Semantic_INVALID_STATE,
	Semantic_ARITY_MISMATCH,
	Semantic_BAD_LITERAL_VALUE
} Semantic;

Semantic check_semantics(Pair* ast);

#endif