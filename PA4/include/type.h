
#ifndef TYPE_H
#define TYPE_H

#include <stdio.h>
#include <stdbool.h>

#include "pair.h"

typedef enum PrimativeType {
	PrimativeType_INT, PrimativeType_VOID, PrimativeType_ARRAY, PrimativeType_POINTER, PrimativeType_FAIL
} PrimativeType;

typedef enum DefinitionType {
	DefinitionType_FUNCTION, DefinitionType_VARIABLE
} DefinitionType;

typedef struct Parameter {
	PrimativeType     type;
	struct Parameter* next;
} Parameter;

typedef struct Function {
	size_t        nparams;
	PrimativeType type;
	Parameter*    params;
	Parameter*    last;
} Function;

typedef struct Type {

	DefinitionType definition;

	union {
		PrimativeType variable;
		Function      function;
	};
} Type;

PrimativeType PrimativeType_of(ASType astype);

Type* Type_new(DefinitionType definition, PrimativeType type);

Type* Type_takes(Type* base, PrimativeType type);

bool PrimativeType_equals(PrimativeType a, PrimativeType b);

bool Type_equals(Type* a, Type* b);

void Type_free(Type* type);

void Type_write(FILE* file, Type const* type, char const* name);

#endif