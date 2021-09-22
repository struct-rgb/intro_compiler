
#include <stdio.h>
#include <assert.h>

#include "../src/type.c"
#include "../src/idtable.c"

int main(int argc, char** argv) {

	IDTable* table = IDTable_new(0);

	Type* a = Type_new(DefinitionType_VARIABLE, PrimativeType_INT);

	if (argc == 1) {

		Type* b = Type_new(DefinitionType_VARIABLE, PrimativeType_ARRAY);

		assert(IDTable_put(table, "cat", a) == IDTableStatus_OK);
		assert(IDTable_put(table, "cat", b) == IDTableStatus_DUPLICATE_KEY);
		assert(IDTable_put(table, "dog", b) == IDTableStatus_OK);
		assert(IDTable_get(table, "cat") == a);
		assert(IDTable_get(table, "dog") == b);

		Type_free(b);

	} else {
		for (int i = 1; i < argc; ++i) {
			IDTable_put(table, argv[i], a);
		}

		for (int i = 1; i < argc; ++i) {
			assert(a == IDTable_get(table, argv[i]));
		}
	}

	IDTable_write(stdout, table);

	Type_free(a);
}