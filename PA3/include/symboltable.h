
#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <stdbool.h>

#include "type.h"
#include "idtable.h"
#include "semantics.h"

typedef struct Scope {
	int           depth;
	struct Scope* parent;
	IDTable*      symbols;
} Scope;

typedef struct SymbolTable {
	int    depth;
	Scope* here;
	Scope* root;
} SymbolTable;

SymbolTable* SymbolTable_new(void);

bool SymbolTable_enter_scope(SymbolTable* table);

void SymbolTable_exit_scope(SymbolTable* table);

Semantic SymbolTable_lookup(SymbolTable* table, char const* string, Type** out_type, Scope** out_scope);

Semantic SymbolTable_define(SymbolTable* table, char const* string, Type* type);

Semantic SymbolTable_global(SymbolTable* table, char const* string, Type* type);

void SymbolTable_free(SymbolTable* table);

#endif