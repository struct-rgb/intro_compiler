
#include <stdlib.h>
#include <stdbool.h>

#include "../include/symboltable.h"

bool SymbolTable_enter_scope(SymbolTable* table) {

	Scope* s = malloc(sizeof (Scope));
	if (!s) goto fail_1;

	s->symbols = IDTable_new(0);
	if (!s->symbols) goto fail_2;

	s->depth    = ++table->depth;
	s->parent   = table->here ? table->here : NULL;
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
	IDTable_put(table->here->symbols, "input", builtin_input);
	IDTable_put(table->here->symbols, "output", builtin_output);

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

Semantic SymbolTable_lookup(SymbolTable* table, char const* string, Type** out_type, Scope** out_scope) {

	Type*  type;
	Scope* scope = table->here;	

	while(scope && !(type = IDTable_get(scope->symbols, string))) {
		scope = scope->parent;
	}

	if (!type || !scope) {
		*out_type  = NULL;
		*out_scope = NULL;
		return Semantic_UNDECLARED_SYMBOL;
	} else {
		*out_type  = type;
		*out_scope = scope;
		return Semantic_OK;
	}
}


Semantic SymbolTable_define(SymbolTable* table, char const* string, Type* type) {

	switch (IDTable_put(table->here->symbols, string, type)) {

		case IDTableStatus_OK:
			return Semantic_OK;

		case IDTableStatus_DUPLICATE_KEY:
			return Semantic_REDECLARATION;

		case IDTableStatus_NO_SPACE:
		default:
			return Semantic_INTERNAL_ERROR;

	}
}

Semantic SymbolTable_global(SymbolTable* table, char const* string, Type* type) {

	switch (IDTable_put(table->root->symbols, string, type)) {

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
