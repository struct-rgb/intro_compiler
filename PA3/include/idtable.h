
#ifndef IDTABLE_H
#define IDTABLE_H

#include <stdio.h>

#include "type.h"

typedef enum IDTableStatus {
	IDTableStatus_OK, IDTableStatus_NO_SPACE, IDTableStatus_DUPLICATE_KEY	
} IDTableStatus;

typedef struct IDTable {
	unsigned     size_class;
	unsigned     used;
	char const** keys;
	Type**       vals;
} IDTable;

IDTable* IDTable_new(int size_class);

IDTableStatus IDTable_put(IDTable* table, char const* key, Type* val);

Type* IDTable_get(IDTable* table, char const* key);

void IDTable_free(IDTable* table);

void IDTable_write(FILE* file, IDTable* table);

#endif


