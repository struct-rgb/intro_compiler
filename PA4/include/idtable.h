
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
	int*         offs;
} IDTable;

IDTable* IDTable_new(int size_class);

IDTableStatus IDTable_put(IDTable* table, char const* key, Type* val, int off);

Type* IDTable_get(IDTable* table, char const* key, int* out_offset);

void IDTable_free(IDTable* table);

void IDTable_write(FILE* file, IDTable* table);

#endif


