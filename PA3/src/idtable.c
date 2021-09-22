
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/type.h"
#include "../include/idtable.h"

const unsigned SIZE_CLASSES = 9;

const unsigned SIZE_CLASS[] = {
	37, 67, 127, 257, 509, 1019, 2039, 4073, 7919
};

IDTable* IDTable_new(int size_class) {

	if (size_class < 0 || size_class > 9)
		return NULL;

	IDTable* table = malloc(sizeof (IDTable));
	if (!table) goto fail_1;

	table->keys = calloc(SIZE_CLASS[size_class], sizeof (char const*));
	if (!table->keys) goto fail_2;

	table->vals = calloc(SIZE_CLASS[size_class], sizeof (Type*));
	if (!table->vals) goto fail_3;

	table->size_class = 0;
	table->used       = 0;

	return table;

fail_3:
	free(table->keys);
fail_2:
	free(table);
fail_1:
	return NULL;
}

static unsigned hash(char const* key, unsigned mod) {

	char     glyph;
	unsigned value = 0;

	while ((glyph = *key++)) {
		value = (31 * value + glyph) % mod;
	}

	return value;
}

static unsigned hash2(char const* key, unsigned mod) {
	
	char     glyph;
	unsigned value = 0;

	while ((glyph = *key++)) {
		value = (17 * value + glyph) % mod;
	}

	return value + 1;
}

/* this assumes the table is large enough to insert */
static IDTableStatus IDTable_put_helper(unsigned mod, char const** keys, Type** vals, char const* key, Type* val) {

	char const* tkey;
	unsigned index  = hash(key, mod);
	unsigned offset = hash2(key, mod);

	while ((tkey = keys[index])) {

		/* key already exists */
		if (strcmp(key, tkey) == 0) {
			return IDTableStatus_DUPLICATE_KEY;
		}

		index = (index + offset) % mod;
	}

	keys[index] = key;
	vals[index] = val;
	return IDTableStatus_OK;
}

static bool IDTable_grow(IDTable* table) {

	/* fail since we have no larger prime numbers */
	if (table->size_class + 1 == SIZE_CLASSES)
		return false;

	unsigned old_size = SIZE_CLASS[table->size_class];
	unsigned new_size = SIZE_CLASS[table->size_class + 1];
	
	char const** keys = calloc(new_size, sizeof (char const*));
	if (!table->keys) goto fail_1;

	Type** vals = calloc(new_size, sizeof (Type*));
	if (!table->vals) goto fail_2;

	for (unsigned i = 0; i < old_size; ++i) {
		if (table->keys[i]) {
			switch (IDTable_put_helper(new_size, keys, vals, table->keys[i], table->vals[i])) {
				
				case IDTableStatus_OK:
					break;
				
				case IDTableStatus_NO_SPACE:
					/* fallthrough */
				case IDTableStatus_DUPLICATE_KEY:
					/* fallthrough */
				default:
					goto fail_3;
			}
		}
	}

	free(table->keys);
	free(table->vals);

	table->keys = keys;
	table->vals = vals;
	++table->size_class;

	/* used should not have changed */
	return true;

fail_3:
	free(vals);
fail_2:
	free(keys);
fail_1:
	return false;
}

static bool IDTable_full(IDTable* table) {
	return (double) (table->used) / (double) (SIZE_CLASS[table->size_class]) > 0.66;
}

IDTableStatus IDTable_put(IDTable* table, char const* key, Type* val) {

	/* if the table cannot be resized, die since this is a compiler */
	if (IDTable_full(table) && !IDTable_grow(table))
		return IDTableStatus_NO_SPACE;

	IDTableStatus status = IDTable_put_helper(
		SIZE_CLASS[table->size_class],
		table->keys,
		table->vals,
		key,
		val
	);

	switch (status) {
		case IDTableStatus_OK:
			++table->used;
		default:
			return status;
	}
}

Type* IDTable_get(IDTable* table, char const* key) {
	
	char const* tkey;
	unsigned mod    = SIZE_CLASS[table->size_class];
	unsigned index  = hash(key, mod);
	unsigned offset = hash2(key, mod);

	while ((tkey = table->keys[index]) && strcmp(key, tkey) != 0) {
		index = (index + offset) % mod;
	}

	return table->vals[index];
}

void IDTable_free(IDTable* table) {

	if (!table) return;

	unsigned size = SIZE_CLASS[table->size_class];

	for (unsigned i = 0; i < size; ++i) {
		Type_free((void*) table->vals[i]);
	}

	free(table->keys);
	free(table->vals);

	free(table);
}

void IDTable_write(FILE* file, IDTable* table) {

	if (!table) return;

	unsigned size = SIZE_CLASS[table->size_class];

	fprintf(file, "size: %u (class %u)\n", size, table->size_class);
	fprintf(file, "used: %u (%.2lf%%)\n", table->used, 100.0 * ((double) table->used/(double) size));

	for (unsigned i = 0; i < size; ++i) {
		fprintf(file, "item #%04u: ", i);
		Type_write(file, table->vals[i], table->keys[i]);
		fputs("\n", file);
	}
}

