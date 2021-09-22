
#include <stdio.h>

#include "../src/type.c"

int main(int argc, char** argv) {

	Type* variable = Type_new(DefinitionType_VARIABLE, PrimativeType_INT);

	Type_write(stdout, variable, "variable");
	puts("\n");

	printf("variable == variable: %d\n\n", Type_equals(variable, variable));

	Type* factorial = Type_takes(
		Type_new(DefinitionType_FUNCTION, PrimativeType_INT), PrimativeType_INT
	);

	Type_write(stdout, factorial, "factorial");
	puts("\n");

	Type* sort = Type_new(DefinitionType_FUNCTION, PrimativeType_VOID);

	Type_takes(sort, PrimativeType_INT);

	Type_takes(sort, PrimativeType_ARRAY);

	Type_write(stdout, sort, "sort");
	puts("\n");

	printf("sort == factorial: %d\n\n", Type_equals(sort, factorial));

	printf("variable == factorial: %d\n\n", Type_equals(variable, factorial));

	Type* mainfn = Type_takes(
		Type_new(DefinitionType_FUNCTION, PrimativeType_VOID), PrimativeType_VOID
	);

	Type_write(stdout, mainfn, "main");
	puts("\n");

	printf("main == main: %d\n\n", Type_equals(mainfn, mainfn));

	Type_free(variable);
	Type_free(factorial);
	Type_free(sort);
	Type_free(mainfn);

}