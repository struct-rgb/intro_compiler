
#include "../include/codegen.h"

typedef enum CGMeta {
	CGMeta_TEXT, CGMeta_DATA
} CGMeta;

static CGMeta segment;

static void codegen_compound_stmt(FILE* file, Pair* ast, char const* function);

static void codegen_expression(FILE* file, Pair* ast);

static char const BUILTINS[]  =
	".text\n"
	"_f_output:\n"
	"  lw $a0, 4($sp)\n"
	"  li $v0, 1\n"
	"  syscall\n"
	"  li $v0, 11\n"
	"  li $a0, 0x0a\n"
	"  syscall\n"
	"  li $a0, 0\n"
	"  jr $ra\n"
	"\n"
	"_f_input:\n"
	"  li $v0, 5\n"
	"  syscall\n"
	"  move $a0, $v0\n"
	"  jr $ra\n"
	"\n"
;

static char const ENTRY_EXIT[] = 
	"main:\n"
	"  jal _f_main\n"
	"  li $v0, 10\n"
	"  syscall\n"
;

static int codegen_call_arguments(FILE* file, Pair* ast) {
	
	if (!ast) return 0;

	int   count = codegen_call_arguments(file, ast->cdr);
	Pair* node  = ast->car;

	/* node->dyn stores whether the variable is subscripted */
	if (node->val == ASType_VAR && !node->dyn) {

		/* special case for array references passed into functions,
		   passing it to codegen_expression would end up derefenceing it,
		   which we don't want to do */

		Pair* var = node->cdr->car;

		switch ((PrimativeType) node->num) {

			case PrimativeType_ARRAY:
				if (var->num == 0) {
					fprintf(file,
						"  la $a0, _v_%s\n",
					var->dyn);
				} else {
					fprintf(file,
						"  addi $a0, $fp, %d\n",
					var->num);
				}
				break;

			case PrimativeType_POINTER:
				fprintf(file,
					"  addi $a0, $fp, %d\n"
					"  lw $a0, 0($a0)\n",
				var->num);
				break;

			default:
				codegen_expression(file, ast->car);
				break;
		}

	} else {
		codegen_expression(file, ast->car);
	}

	/* push argument onto stack */
	fputs(
		"  sw $a0, 0($sp)\n"
		"  addiu $sp, $sp, -4\n",
	file);

	return count + 1;
}

static void codegen_variable_address(FILE* file, Pair* ast) {

	Pair* node;

	node = ast->cdr;
	int offset = node->car->num;

	/* variable is marked indexable */
	if (ast->dyn) {
		/* compute index */
		codegen_expression(file, node->cdr->car);

		/* shift index to word size and save in $t1 */
		fputs(
			"  sll $a0, $a0, 2\n"
			"  move $t1, $a0\n",
		file);
	}

	switch (ast->num) {

		case PrimativeType_POINTER:
			/* variable is a pointer parameter */

			/* calculate stack offset of argument
			   and load the */
			fprintf(file,
				"  addi $a0, $fp, %d\n"
				"  lw $a0, 0($a0)\n",
			offset);
			break;

		default:
			if (offset == 0) {
				/* variable is a global */

				/* load address of label */
				fprintf(file,
					"  la $a0, _v_%s\n",
				node->car->dyn);
			} else {
				/* variable is a local */

				/* calculate stack offset */
				fprintf(file,
					"  addi $a0, $fp, %d\n",
				offset);
			}
			break;

	}

	if (ast->dyn) {
		/* add index to address */
		fputs("  add $a0, $t1, $a0\n", file);
	}
}

static inline void codegen_operator(FILE* file, Pair* ast, char const* operator) {
	Pair* node = ast->cdr;
	codegen_expression(file, node->car);
	fputs(
		"  sw $a0, 0($sp)\n"
		"  addiu $sp, $sp, -4\n",
	file);
	node = node->cdr;
	codegen_expression(file, node->car);
	fprintf(file,
		"  lw $t1, 4($sp)\n"
		"  %s $a0, $t1, $a0\n"
		"  addiu $sp, $sp, 4\n",
	operator);
}

static inline void codegen_factor(FILE* file, Pair* ast, char const* operator) {
	Pair* node = ast->cdr;
	codegen_expression(file, node->car);
	fputs(
		"  sw $a0, 0($sp)\n"
		"  addiu $sp, $sp, -4\n",
	file);
	node = node->cdr;
	codegen_expression(file, node->car);
	fprintf(file,
		"  lw $t1, 4($sp)\n"
		"  %s $t1, $a0\n"
		"  mflo $a0\n"
		"  addiu $sp, $sp, 4\n",
	operator);
}

static void codegen_expression(FILE* file, Pair* ast) {

	Pair* node;

	switch (ast->val) {

		case ASType_NUM:
			fprintf(file, "  li $a0, %d\n", ast->num);
			break;

		case ASType_LE:
			codegen_operator(file, ast, "sle");
			break;

		case ASType_LT:
			codegen_operator(file, ast, "slt");
			break;
		
		case ASType_GT:
			codegen_operator(file, ast, "sgt");
			break;

		case ASType_GE:
			codegen_operator(file, ast, "sge");
			break;

		case ASType_EQ:
			codegen_operator(file, ast, "seq");
			break;

		case ASType_NE:
			codegen_operator(file, ast, "sne");
			break;

		case ASType_ADD:
			codegen_operator(file, ast, "add");
			break;

		case ASType_SUB:
			codegen_operator(file, ast, "sub");
			break;

		case ASType_MUL:
			codegen_factor(file, ast, "mult");
			break;

		case ASType_DIV:
			codegen_factor(file, ast, "div");
			break;

		case ASType_CALL: {

			node = ast->cdr;

			char const* identifier = node->car->dyn;

			/* save frame pointer */
			fputs(
				"  sw $fp, 0($sp)\n"
				"  addiu $sp, $sp, -4\n",
			file);

			node = node->cdr->car;

			int nargs = node->cdr ? codegen_call_arguments(file, node->cdr) : 0;

			/* jump then restore frame pointer */
			fprintf(file,
				"  jal _f_%s\n"
				"  addiu $sp, $sp, %d\n"
				"  lw $fp, 0($sp)\n",
			identifier, (nargs + 1) << 2);

			break;
		}

		case ASType_VAR:
			/* generate address of data */
			codegen_variable_address(file, ast);
			/* dereference address */
			fputs("  lw $a0, 0($a0)\n", file);
			break;

		case ASType_SET:
			node = ast->cdr;
			codegen_variable_address(file, node->car);
			fputs(
				"  sw $a0, 0($sp)\n"
				"  addiu $sp, $sp, -4\n",
			file);
			node = node->cdr;
			codegen_expression(file, node->car);
			fputs(
				"  lw $t1, 4($sp)\n"
				"  sw $a0, 0($t1)\n"
				"  addiu $sp, $sp, 4\n",
			file);
			break;

		default:
			return;
	}
}



static void codegen_statment(FILE* file, Pair* ast, char const* function) {

	Pair* node;

	static int label_counter;

	switch (ast->val) {

		 /* expression statements */
        case ASType_NUM:
        case ASType_LE:
        case ASType_LT:
        case ASType_GT:
        case ASType_GE:
        case ASType_EQ:
        case ASType_NE:
        case ASType_ADD:
        case ASType_SUB:
        case ASType_MUL:
        case ASType_DIV:
        case ASType_CALL:
        case ASType_VAR:
        case ASType_SET:
        	codegen_expression(file, ast);
        	break;

		case ASType_EMPTY_STMT:
			/* nothing to do here */
			break;

		case ASType_RETURN_STMT:
			node = ast->cdr;
			if (node) codegen_expression(file, node->car);
			fprintf(file, "  j _f_%s_exit\n", function);
			break;

		case ASType_ITERATION_STMT: {

			/* get unique identifier for this statement */
			int label = label_counter++;

			node = ast->cdr;

			fprintf(file, "_while_%d:\n", label);

			 /* generate code for test expression */
			codegen_expression(file, node->car);

			fprintf(file, "  beq $a0, $zero, _end_while_%d\n", label);

			/* generate statements for loop body */
			node = node->cdr;
			codegen_statment(file, node->car, function);

			fprintf(file,
				"  b _while_%d\n"
				"_end_while_%d:\n",
			label, label);

			break;
		}

		case ASType_SELECTION_STMT: {

			/* get unique identifier for this statement */
			int label = label_counter++;

			node = ast->cdr;

			 /* generate code for test expression */
			codegen_expression(file, node->car);

			if (node->cdr->cdr) {
				/* two branch if statement */
				fprintf(file, "  bne $a0, $zero, _if_%d\n", label);

				node = node->cdr;
				Pair* true_branch  = node->car;
				node = node->cdr; 
				Pair* false_branch = node->car;

				/* generate false branch statements */
				codegen_statment(file, false_branch, function);

				fprintf(file,
					"  b _end_if_%d\n"
					"_if_%d:\n",
				label, label);

				/* generate true branch statements */
				codegen_statment(file, true_branch, function);

			} else {
				/* one branch if statement */
				fprintf(file, "  beq $a0, $zero, _end_if_%d\n", label);

				/* generate true branch statements */
				node = node->cdr;
				codegen_statment(file, node->car, function);
			}

			fprintf(file, "_end_if_%d:\n", label);
			break;
		}

		case ASType_COMPOUND_STMT:
			codegen_compound_stmt(file, ast, function);
			break;

		default:
			return;
	}

}

static void codegen_compound_stmt(FILE* file, Pair* ast, char const* function) {

	int offset = ast->num;

	Pair* node = ast->cdr;
	if (!node) return;

	if (offset)
		fprintf(file, "  addiu $sp, $sp, %d\n", offset);

	 do {

		switch (node->car->val) {

			case ASType_NUM:
			case ASType_LE:
			case ASType_LT:
			case ASType_GT:
			case ASType_GE:
			case ASType_EQ:
			case ASType_NE:
			case ASType_ADD:
			case ASType_SUB:
			case ASType_MUL:
			case ASType_DIV:
			case ASType_CALL:
			case ASType_VAR:
			case ASType_SET:
				codegen_expression(file, node->car);
				break;

			case ASType_EMPTY_STMT:
			case ASType_RETURN_STMT:
			case ASType_ITERATION_STMT:
			case ASType_SELECTION_STMT:
			case ASType_COMPOUND_STMT:
				codegen_statment(file, node->car, function);
				break;


			case ASType_VAR_DECLARATION:
				/* nothing to do, offset was calculated during semantic analysis */
				break;

			default:
				return;
		}

	} while ((node = node->cdr));

	if (offset)
		fprintf(file, "  addiu $sp, $sp, %d\n", -offset);
}

static void codegen_fun_declaration(FILE* file, Pair* ast) {

	if (segment != CGMeta_TEXT) {
		segment = CGMeta_TEXT;
		fputs("\n.text\n", file);
	}

	Pair*       node       = ast->cdr->cdr;
	char const* identifier = node->car->dyn;

	fprintf(file,
		"_f_%s:\n"
		"  sw $ra, 0($sp)\n"
		"  move $fp, $sp\n"
		"  addiu $sp, $sp, -4\n",
	identifier);

	node = node->cdr->cdr;

	codegen_compound_stmt(file, node->car, identifier);

	fprintf(file,
		"_f_%s_exit:\n"
		"  move $sp, $fp\n"
		"  lw $ra, 0($sp)\n"
		"  jr $ra\n\n",
	identifier);

}

/* this function is only used for global variables
   local variables are allocate to the stack at the
   beginning of their corresponding compound statement */

static void codegen_var_declaration(FILE* file, Pair* ast) {

	if (segment != CGMeta_DATA) {
		segment = CGMeta_DATA;
		fputs("\n.data\n", file);
	}

	Pair*       node       = ast->cdr->cdr;
	char const* identifier = node->car->dyn;
	int         size       = (node = node->cdr) ? node->car->num << 2 : 4; 

	fprintf(file, "_v_%s: .space %d\n", identifier, size);
}

/*     !!! WARNING !!!
   AST MUST PASS SEMANTIC
   ANALYSIS BEFORE CODEGEN */

void codegen(FILE* file, Pair* ast) {

	Pair* node = ast;
	segment    = CGMeta_TEXT;

	fputs(BUILTINS, file);

	/* skip program node to get into declaration list */
	node = node->cdr;

	while (node) {

		Pair* car = node->car;
		
		/* generate code for this node */
		switch (car->val) {

			case ASType_FUN_DECLARATION:
				codegen_fun_declaration(file, car);
				break;

			case ASType_VAR_DECLARATION:
				codegen_var_declaration(file, car);
				break;

			default:
				return;
		}

		node = node->cdr;
	}

	if (segment != CGMeta_TEXT) {
		segment = CGMeta_TEXT;
		fputs("\n.text\n", file);
	}

	fputs(ENTRY_EXIT, file);
}


