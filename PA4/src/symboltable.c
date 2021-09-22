
#include <stdlib.h>
#include <stdbool.h>

#include "../include/symboltable.h"

bool SymbolTable_enter_scope(SymbolTable* table) {

	Scope* s = malloc(sizeof (Scope));
	if (!s) goto fail_1;

	s->symbols = IDTable_new(0);
	if (!s->symbols) goto fail_2;

	s->depth    = ++table->depth;
	s->parent   = table->here ? table->here       : NULL;
	s->varmax   = s->parent   ? s->parent->varmax : 0;
	s->parmax   = 0;
	table->here = s;

	return true;

fail_2:
	free(s);
fail_1:
	return false;
}

SymbolTable* SymbolTable_new(void) {

	SymbolTable* table = malloc(sizeof (SymbolTable));
	if (!table) goto fail_1;

	table->depth = -1;
	table->here  = NULL;

	if (!SymbolTable_enter_scope(table))
		goto fail_2;

	/* set the global scope to the current scope */
	table->root = table->here;

	Type* builtin_input = Type_takes(
		Type_new(DefinitionType_FUNCTION, PrimativeType_INT), PrimativeType_VOID
	);

	if (!builtin_input) goto fail_3;

	Type* builtin_output = Type_takes(
		Type_new(DefinitionType_FUNCTION, PrimativeType_VOID), PrimativeType_INT
	);

	if (!builtin_output) goto fail_3;

	/*
	 * Should not run out of room since these are the first inputs
	 * Should be no duplicate keys for the same reason
	 * Therefore: not checking return value of these insertions
	 */
	IDTable_put(table->here->symbols, "input", builtin_input, 0);
	IDTable_put(table->here->symbols, "output", builtin_output, 0);

	return table;

fail_3:
	SymbolTable_exit_scope(table);
fail_2:
	free(table);
fail_1:
	return NULL;
}

void SymbolTable_exit_scope(SymbolTable* table) {

	Scope* there = table->here;
	table->here  = there->parent;

	IDTable_free(there->symbols);
	free(there);
};

Semantic SymbolTable_lookup(SymbolTable* table, char const* string, Type** out_type, int* out_offset) {

	Type*  type;
	Scope* scope = table->here;	

	while(scope && !(type = IDTable_get(scope->symbols, string, out_offset))) {
		scope = scope->parent;
	}

	if (!type || !scope) {
		*out_type   = NULL;
		if (out_offset) *out_offset = 0;
		return Semantic_UNDECLARED_SYMBOL;
	} else {
		*out_type   = type;
		return Semantic_OK;
	}
}


Semantic SymbolTable_define(SymbolTable* table, char const* string, Type* type, int off) {

	int offset   = 0;
	Scope* scope = table->here;

	if (scope != table->root) {
		if (off < 0) {
			/* local variable branch */
			scope->varmax += off;
			offset = scope->varmax;
		} else if (off == 4) {
			/* parameter branch */
			scope->parmax += off;
			offset = scope->parmax;
		} else {
			/* invalid branch */
			return Semantic_INTERNAL_ERROR;
		}
	}

	switch (IDTable_put(scope->symbols, string, type, offset)) {

		case IDTableStatus_OK:
			return Semantic_OK;

		case IDTableStatus_DUPLICATE_KEY:
			return Semantic_REDECLARATION;

		case IDTableStatus_NO_SPACE:
		default:
			return Semantic_INTERNAL_ERROR;

	}
}

Semantic SymbolTable_function(SymbolTable* table, char const* string, Type* type) {

	switch (IDTable_put(table->root->symbols, string, type, 0)) {

		case IDTableStatus_OK:
			return Semantic_OK;

		case IDTableStatus_DUPLICATE_KEY:
			return Semantic_REDECLARATION;

		case IDTableStatus_NO_SPACE:
		default:
			return Semantic_INTERNAL_ERROR;

	}
}

void SymbolTable_free(SymbolTable* table) {

	if (!table) return;

	while (table->here) {
		SymbolTable_exit_scope(table);
	}

	free(table);
};
