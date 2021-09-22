
#include <stdio.h>
#include <stdlib.h>

#include "../include/type.h"

PrimativeType PrimativeType_of(ASType astype) {
	switch (astype) {
		case ASType_INT:
			return PrimativeType_INT;
		case ASType_VOID:
			return PrimativeType_VOID;
		case ASType_POINTER:
			return PrimativeType_POINTER;
		default:
			return PrimativeType_FAIL;
	}
}

Type* Type_new(DefinitionType definition, PrimativeType type) {

	Type* t = malloc(sizeof (Type));
	if (!t) return NULL;

	t->definition = definition;

	switch (definition) {
		case DefinitionType_VARIABLE:
			t->variable = type;
			break;
		case DefinitionType_FUNCTION:
			t->function.nparams = 0;
			t->function.type    = type;
			t->function.params  = NULL;
			t->function.last    = NULL;
			break;
	}

	return t;
}

Type* Type_takes(Type* base, PrimativeType type) {

	if (!base) return NULL;

	if (base->definition != DefinitionType_FUNCTION)
		goto fail;

	Parameter* p = malloc(sizeof (Parameter));
	if (!p) goto fail;

	p->type = type;
	p->next = NULL;

	Function* function = &base->function;

	if (function->params == NULL) {
		function->params = p;
		function->last   = p;
	} else {
		function->last->next = p;
		function->last       = p;
	}

	++function->nparams;
	return base;

fail:
	Type_free(base);
	return NULL;
}

bool PrimativeType_equals(PrimativeType a, PrimativeType b) {
	return (a == PrimativeType_ARRAY && b == PrimativeType_POINTER) || (a == PrimativeType_POINTER && b == PrimativeType_ARRAY)
		? true
		: a == b;
}

bool Type_equals(Type* a, Type* b) {

	/* a function is not a variable etc */
	if (a->definition != b->definition)
		return false;

	switch (a->definition) {

		case DefinitionType_VARIABLE:
			return a->variable == b->variable;

		case DefinitionType_FUNCTION: {
			
			Parameter *x, *y;

			/* check return type equivalence */
			if (!PrimativeType_equals(a->function.type, b->function.type))
				return false;

			/* check arity */
			if (a->function.nparams != b->function.nparams)
				return false;

			/* check each parameter */
			x = a->function.params;
			y = b->function.params;

			while (x && y) {
				if (!PrimativeType_equals(x->type, y->type))
					return false;

				x = x->next;
				y = y->next;
			}

			return true;
		}

		default:
			return false;
	}
}

void Type_free(Type* type) {

	if (!type) return;

	switch (type->definition) {

		case DefinitionType_FUNCTION: {

			Parameter *p = type->function.params, *n;

			while (p) {
				n = p->next;
				
				free(p);

				p = n;
			}
		}		
		/* fallthrough */

		case DefinitionType_VARIABLE:
			free(type);
			break;	
	}
}

static char const* PrimativeType_string(PrimativeType type) {
	switch (type) {
		
		case PrimativeType_INT:
			return "int";

		case PrimativeType_VOID:
			return "void";

		case PrimativeType_ARRAY:
			return "int[#]";

		case PrimativeType_POINTER:
			return "int[]";

		default:
			return "???";
	}
}

void Type_write(FILE* file, Type const* type, char const* name) {

	if (!type) return;

	switch (type->definition) {

		case DefinitionType_VARIABLE:
			fprintf(file, "%s %s;", PrimativeType_string(type->variable), name ? name : "x");
			break;	

		case DefinitionType_FUNCTION: {

			fprintf(file, "%s %s(", PrimativeType_string(type->function.type), name ? name : "x");

			Parameter *p = type->function.params;

			while (p) {

				fprintf(file, "%s%s", PrimativeType_string(p->type), p->next ? ", " : ");");
				p = p->next;
			}
		}
		break;
	}

}
