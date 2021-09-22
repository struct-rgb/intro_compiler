
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "../include/pair.h"
#include "../include/parser.h"

static Pair* p_var_declaration();
static Pair* p_compound_stmt();
static Pair* p_statement();

static Pair* p_expression();

static TokenList* token;

// static void look(size_t depth) {

// 	TokenList* list = token;

// 	for (size_t i = 0; list && i < depth; ++i) {
// 		list = list->next;
// 	}

// 	fprintf(stderr, list ? "\"%s\"\n" : "%s", list ? list->value.lexeme->buffer : "(none)");
// }

static void write_spaces(FILE* file, size_t spaces) {
	for (size_t i = 0; i < spaces; ++i) {
		fputc(' ', file);
	}
}

static void write_ast_rec(FILE* file, Pair* root, size_t indent) {
	if (root->val) {
		write_spaces(file, indent);
		fprintf(file, "[%s", root->val);
	}

	if (root->car) {
		fputs("\n", file);
		write_ast_rec(file, root->car, indent + 2);
	}

	if (root->cdr) {
		write_ast_rec(file, root->cdr, indent);
	} else {
		fputs("]", file);
	}
}

void write_ast(FILE* file, Pair* root) {

	if (!root) {
		fputs("", file);
		return;
	}

	write_ast_rec(file, root, 0);
}


static bool is_terminal(char const* terminal) {

	return token && strcmp(token->value.lexeme->buffer, terminal) == 0;
}

#define INITPAIRS(NUMBER)          \
	int    index         = 0;      \
	Pair*  pairs[NUMBER] = {NULL}  \

#define CAPTURE(PRODUCTION)      \
	pairs[index] = (PRODUCTION); \
	if (pairs[index]) {          \
		if (token) {             \
			token = token->next; \
		}                        \
		++index;                 \
	} else {                     \
    	goto failure;            \
	}

#define NOPTURE(PRODUCTION)      \
	pairs[index] = (PRODUCTION); \
	if (pairs[index]) {          \
		++index;                 \
	} else {                     \
    	goto failure;            \
	}

#define EXPECTV(TERMINAL)         \
	if (!is_terminal(TERMINAL)) { \
		goto failure;             \
	} else {                      \
		if (token) {              \
			token = token->next;  \
		}                         \
	}

#define CLEANUP()                \
failure:                         \
	--index;                     \
	while (index >= 0) {         \
		Pair_free(pairs[index]); \
		--index;                 \
	}                            \
	return NULL

#define INITMATCH()          \
	Pair* out;               \
	TokenList* save = token; \

#define ATTEMPT(PRODUCTION)     \
	token = save;               \
	if ((out = (PRODUCTION))) { \
		return out;             \
	}

/* <? any single terminal ?> */
static Pair* p_terminal(char const* terminal) {
	if (!is_terminal(terminal))
		return NULL;

	return Pair_new(terminal, NULL, NULL);
}

/* <type-specifier> ::= int | void */
static Pair* p_type_specifier() {

	Pair* out;
	if ((out = p_terminal("int")))  return out;
	if ((out = p_terminal("void"))) return out;
	return NULL;
}

/* ID */
static Pair* p_identifier() {

	if (!token) return NULL;

	if (token->value.symbol == TokenType_ID) {
		return Pair_dyn(token->value.lexeme->buffer, NULL, NULL);
	} else {
		return NULL;
	}
}

/* NUM */
static Pair* p_number() {

	if (!token) return NULL;

	if (token->value.symbol == TokenType_NUM) {
		return Pair_dyn(token->value.lexeme->buffer, NULL, NULL);
	} else {
		return NULL;
	}
}

/* <relop> ::= <= | < | > | >= | == | != */
static Pair* p_relop() {

	Pair* out;
	if ((out = p_terminal("<=")))  return out;
	if ((out = p_terminal("<")))   return out;
	if ((out = p_terminal(">")))   return out;
	if ((out = p_terminal(">=")))  return out;
	if ((out = p_terminal("==")))  return out;
	if ((out = p_terminal("!=")))  return out;
	return NULL;
}

/* <addop> ::= + | - */
static Pair* p_addop() {

	Pair* out;
	if ((out = p_terminal("+")))  return out;
	if ((out = p_terminal("-")))  return out;
	return NULL;
}

/* <mulop> ::= * | / */
static Pair* p_mulop() {

	Pair* out;
	if ((out = p_terminal("*")))  return out;
	if ((out = p_terminal("/")))  return out;
	return NULL;
}

/* <arg-list> ::= <arg-list> , <expression> | <expression> */
static Pair* p_arg_list() {

	Pair* start = NULL;
	Pair* cur   = NULL;

	Pair* car = p_expression();
	if (!car) return NULL;

	start = Pair_new(NULL, car, NULL);
	cur   = start;

	while (token) {

		if (!is_terminal(",")) {
			return start;
		} else {
			if (token) token = token->next;
		}

		car = p_expression();
		if (car) {
			cur->cdr = Pair_new(NULL, car, NULL);
			cur      = cur->cdr;
		} else {
			return NULL;
		}
	}

	return NULL;
}

/* <args> ::= <arg-list> | empty */
static Pair* p_args() {
	return Pair_new("args", NULL, p_arg_list());
}

/* <call> ::= ID ( <args> ) */
static Pair* p_call() {
	INITPAIRS(2);

	CAPTURE(p_identifier());
	EXPECTV("(");
	NOPTURE(p_args());
	EXPECTV(")");

	return Pair_new("call", NULL, Pair_new(NULL, pairs[0], Pair_new(NULL, pairs[1], NULL)));

	CLEANUP();
}

/* ID [ <expression> ] */
static Pair* p_var_1() {
	INITPAIRS(2);

	CAPTURE(p_identifier());
	EXPECTV("[");
	NOPTURE(p_expression());
	EXPECTV("]");

	return Pair_new("var", NULL, Pair_new(NULL, pairs[0], Pair_new(NULL, pairs[1], NULL)));

	CLEANUP();
}

/* ID */
static Pair* p_var_2() {
	INITPAIRS(2);

	CAPTURE(p_identifier());

	return Pair_new("var", NULL, Pair_new(NULL, pairs[0], NULL));

	CLEANUP();
}

/* <var> ::= ID | ID [ <expression> ] */
static Pair* p_var() {
	INITMATCH();

	ATTEMPT(p_var_1());
	ATTEMPT(p_var_2());
	return NULL;
}

/* ( <expression> ) */
static Pair* p_factor_1() {
	INITPAIRS(1);

	EXPECTV("(");
	NOPTURE(p_expression());
	EXPECTV(")");

	return pairs[0];

	CLEANUP();
}

/* <call> */
static Pair* p_factor_3() {
	INITPAIRS();

	NOPTURE(p_call());

	return pairs[0];

	CLEANUP();
}

/* <var> */
static Pair* p_factor_2() {
	INITPAIRS();

	NOPTURE(p_var());

	return pairs[0];

	CLEANUP();
}

/* NUM */
static Pair* p_factor_4() {
	INITPAIRS(1);

	CAPTURE(p_number());

	return pairs[0];

	CLEANUP();
}

/* <factor> ::= ( <expression> ) | <var> | <call> | NUM */
static Pair* p_factor() {
	INITMATCH();

	ATTEMPT(p_factor_1());
	ATTEMPT(p_factor_3());
	ATTEMPT(p_factor_2());
	ATTEMPT(p_factor_4());
	return NULL;
}

/* <term> ::= <term> <mulop> <factor> | <factor> */
static Pair* p_term() {
	Pair* oper, * carA, * carB;

	carA = p_factor();
	if (!carA) return NULL;

	while (token) {

		if ((oper = p_mulop())) {
			if (token) token = token->next;
		} else {
			return carA;
		}

		carB = p_factor();
		if (carB) {
			oper->cdr = Pair_new(NULL, carA, Pair_new(NULL, carB, NULL));
			carA      = oper;
		} else {
			return NULL;
		}
	}

	return NULL;
}

/* <additive-expression> ::= <additive-expression> <addop> <term> | <term> */
static Pair* p_additive_expression() {
	Pair* oper, * carA, * carB;

	carA = p_term();
	if (!carA) return NULL;

	while (token) {

		if ((oper = p_addop())) {
			if (token) token = token->next;
		} else {
			return carA;
		}

		carB = p_term();
		if (carB) {
			oper->cdr = Pair_new(NULL, carA, Pair_new(NULL, carB, NULL));
			carA      = oper;
		} else {
			return NULL;
		}
	}

	return NULL;
}

/* <additive-expression> <relop> <additive-expression> */
static Pair* p_simple_expression_1() {
	INITPAIRS(3);

	NOPTURE(p_additive_expression());
	CAPTURE(p_relop());
	NOPTURE(p_additive_expression());

	pairs[1]->cdr = Pair_new(NULL, pairs[0], Pair_new(NULL, pairs[2], NULL));
	return pairs[1];

	CLEANUP();
}

/* <additive-expression> */
static Pair* p_simple_expression_2() {
	INITPAIRS(1);

	NOPTURE(p_additive_expression());

	return pairs[0];

	CLEANUP();
}

/* <simple-expression> ::= <additive-expression> <relop> <additive-expression> 

                       | <additive-expression>*/
static Pair* p_simple_expression() {
	INITMATCH();

	ATTEMPT(p_simple_expression_1());
	ATTEMPT(p_simple_expression_2());
	return NULL;
}

/* <var> = <expression> */
static Pair* p_expression_1() {
	INITPAIRS(2);

	NOPTURE(p_var());
	EXPECTV("=");
	NOPTURE(p_expression());

	return Pair_new("=", NULL, Pair_new(NULL, pairs[0], Pair_new(NULL, pairs[1], NULL)));

	CLEANUP();
}

/* <expression> ::= <var> = <expression> | <simple-expression> */
static Pair* p_expression() {
	INITMATCH();

	ATTEMPT(p_expression_1());
	ATTEMPT(p_simple_expression());
	return NULL;
}

/* <expression> ; */
static Pair* p_expression_stmt_2() {
	INITPAIRS(1);

	NOPTURE(p_expression());
	EXPECTV(";");

	return pairs[0];

	CLEANUP();
}

/* ; */
static Pair* p_expression_stmt_1() {
	INITPAIRS(1);

	CAPTURE(p_terminal(";"))
	
	return pairs[0];

	CLEANUP();
}

/* <expression-stmt> ::= <expression> ; | ; */
static Pair* p_expression_stmt() {
	INITMATCH();

	ATTEMPT(p_expression_stmt_1());
	ATTEMPT(p_expression_stmt_2());
	return NULL;
}

/* return <expression> ; */
static Pair* p_return_stmt_2() {
	INITPAIRS(1);

	EXPECTV("return");
	NOPTURE(p_expression());
	EXPECTV(";");

	return Pair_new("return-stmt", NULL, Pair_new(NULL, pairs[0], NULL));

	CLEANUP();
}

/* return ; */
static Pair* p_return_stmt_1() {
	INITPAIRS(1);

	EXPECTV("return");
	EXPECTV(";");

	return Pair_new("return-stmt", NULL, NULL);

	CLEANUP();
}

/* <return-stmt> ::= return ; | return <expression> ; */
static Pair* p_return_stmt() {
	INITMATCH();

	ATTEMPT(p_return_stmt_1());
	ATTEMPT(p_return_stmt_2());
	return NULL;
}

/* <iteration-stmt> ::= while ( <expression> ) <statement> */
static Pair* p_iteration_stmt() {
	INITPAIRS(2);

	EXPECTV("while");
	EXPECTV("(");
	NOPTURE(p_expression());
	EXPECTV(")");
	NOPTURE(p_statement());

	return Pair_new("iteration-stmt", NULL, Pair_new(NULL, pairs[0], Pair_new(NULL, pairs[1], NULL)));

	CLEANUP();
}

/* if ( <expression> ) <statement> else <statement> */
static Pair* p_selection_stmt_1() {
	INITPAIRS(3);

	EXPECTV("if");
	EXPECTV("(");
	NOPTURE(p_expression());
	EXPECTV(")");
	NOPTURE(p_statement());
	EXPECTV("else");
	NOPTURE(p_statement());

	return Pair_new("selection-stmt", NULL, Pair_new(NULL, pairs[0], Pair_new(NULL, pairs[1], Pair_new(NULL, pairs[2], NULL))));

	CLEANUP();
}

/* if ( <expression> ) <statement> */
static Pair* p_selection_stmt_2() {
	INITPAIRS(2);

	EXPECTV("if");
	EXPECTV("(");
	NOPTURE(p_expression());
	EXPECTV(")");
	NOPTURE(p_statement());

	return Pair_new("selection-stmt", NULL, Pair_new(NULL, pairs[0], Pair_new(NULL, pairs[1], NULL)));

	CLEANUP();
}

/* <selection-stmt> ::= if ( <expression> ) <statement> 

                  | if ( <expression> ) <statement> else <statement> */
static Pair* p_selection_stmt() {
	INITMATCH();

	ATTEMPT(p_selection_stmt_1());
	ATTEMPT(p_selection_stmt_2());
	return NULL;
}

/* <statement> ::= <expression-stmt> | <compound-stmt> | <selection-stmt> 

              | <iteration-stmt> | <return-stmt> */
static Pair* p_statement() {
	INITMATCH();

	ATTEMPT(p_expression_stmt());
	ATTEMPT(p_compound_stmt());
	ATTEMPT(p_selection_stmt());
	ATTEMPT(p_iteration_stmt());
	ATTEMPT(p_return_stmt());
	return NULL;
}

/* <statement-list> ::= <statement-list> <statement> | empty */
static Pair* p_statement_list() {
	Pair* car;
	Pair* start = NULL;
	Pair* cur   = NULL;

	while (token && (car = p_statement())) {
		if (start) {
			cur->cdr = Pair_new(NULL, car, NULL);
			cur      = cur->cdr;
		} else {
			start    = Pair_new(NULL, car, NULL);
			cur      = start;
		}
	}

	return start;
}

/* <local-declarations> ::= <local-declarations> <var-declaration> | empty */
static Pair* p_local_declarations() {
	Pair* car;
	Pair* start = NULL;
	Pair* cur   = NULL;

	while (token && (car = p_var_declaration())) {
		if (start) {
			cur->cdr = Pair_new(NULL, car, NULL);
			cur      = cur->cdr;
		} else {
			start    = Pair_new(NULL, car, NULL);
			cur      = start;
		}
	}

	return start;
}

/* <type-specifier> ID [ ] */
static Pair* p_param_1() {
	INITPAIRS(2);

	CAPTURE(p_type_specifier());
	CAPTURE(p_identifier());
	EXPECTV("[");
	EXPECTV("]");

	return Pair_new("param", NULL, Pair_new(NULL, pairs[0], Pair_new(NULL, pairs[1], Pair_new(NULL, Pair_new("\\[\\]", NULL, NULL), NULL))));

	CLEANUP();
}

/* <type-specifier> ID */
static Pair* p_param_2() {
	INITPAIRS(2);

	CAPTURE(p_type_specifier());
	CAPTURE(p_identifier());

	return Pair_new("param", NULL, Pair_new(NULL, pairs[0], Pair_new(NULL, pairs[1], NULL)));

	CLEANUP();
}

/* <param> ::= <type-specifier> ID | <type-specifier> ID [ ] */
static Pair* p_param() {
	INITMATCH();
	
	ATTEMPT(p_param_1());
	ATTEMPT(p_param_2());
	return NULL;
}

/* <param-list> ::= <param-list> , <param> | <param> */
static Pair* p_param_list() {

	Pair* start = NULL;
	Pair* cur   = NULL;

	Pair* car = p_param();
	if (!car) return NULL;

	start = Pair_new(NULL, car, NULL);
	cur   = start;

	while (token) {

		if (!is_terminal(",")) {
			return start;
		} else {
			if (token) token = token->next;
		}

		car = p_param();
		if (car) {
			cur->cdr = Pair_new(NULL, car, NULL);
			cur      = cur->cdr;
		} else {
			return NULL;
		}
	}

	return NULL;
}

/* <params> ::= <param-list> | void */
static Pair* p_params() {
	Pair* out;
	TokenList* save = token;
	
	token = save;
	if ((out = p_param_list())) {

		return Pair_new("params", NULL, out);
	}
	token = save;
	if (is_terminal("void")) {
		if (token) token = token->next;
		return Pair_new("params", NULL, NULL);
	}
	return NULL;
}

/* <compound-stmt> ::= { <local-declarations> <statement-list> } */
static Pair* p_compound_stmt() {
	INITPAIRS(2);

	TokenList* save;

	EXPECTV("{");
	
	save = token;
	pairs[0] = p_local_declarations();
	++index;
	
	/* if the result is NULL there were no declarations so then token should not have advanced */
	if (!pairs[0] && token != save) goto failure;
	
	save = token;
	pairs[1] = p_statement_list();
	++index;
	
	/* same as above, the token having advanced on a NULL return is an error*/
	if (!pairs[1] && token != save) goto failure;
	
	EXPECTV("}");

	Pair* list = pairs[0]
		? pairs[0]
		: pairs[1]
			? pairs[1]
			: NULL;

	if (list != NULL && list == pairs[0]) {
		/* join the two lists together */
		Pair_last(pairs[0])->cdr = pairs[1];
	}

	return Pair_new("compound-stmt", NULL, list);

	CLEANUP();
}

/* <fun-declaration> ::= <type-specifier> ID ( <params> ) <compound-stmt> */
static Pair* p_fun_declaration() {
	INITPAIRS(4);

	CAPTURE(p_type_specifier());
	CAPTURE(p_identifier());
	EXPECTV("(");
	NOPTURE(p_params());
	EXPECTV(")");
	NOPTURE(p_compound_stmt());

	return Pair_new("fun-declaration", NULL, Pair_new(NULL, pairs[0], Pair_new(NULL, pairs[1], Pair_new(NULL, pairs[2], Pair_new(NULL, pairs[3], NULL)))));

	CLEANUP();
}

/* <type-specifier> ID [ NUM ] ; */
static Pair* p_var_declaration_2() {
	INITPAIRS(3);
	
	CAPTURE(p_type_specifier());
	CAPTURE(p_identifier());
	EXPECTV("[");
	CAPTURE(p_number());
	EXPECTV("]");
	EXPECTV(";");

	return Pair_new("var-declaration", NULL, Pair_new(NULL, pairs[0], Pair_new(NULL, pairs[1], Pair_new(NULL, pairs[2], NULL))));

	CLEANUP();
}

/* <type-specifier> ID ; */
static Pair* p_var_declaration_1() {
	INITPAIRS(2);
	
	CAPTURE(p_type_specifier());
	CAPTURE(p_identifier());
	EXPECTV(";");

	return Pair_new("var-declaration", NULL, Pair_new(NULL, pairs[0], Pair_new(NULL, pairs[1], NULL)));

	CLEANUP();
}

/* <var-declaration> ::= <type-specifier> ID ; | <type-specifier> ID [ NUM ] ; */
static Pair* p_var_declaration() {

	Pair* out;
	TokenList* save = token;
	
	token = save;
	if ((out = p_var_declaration_1())) return out;
	token = save;
	if ((out = p_var_declaration_2())) return out;
	return NULL;
}

/* <declaration> ::= <var-declaration> | <fun-declaration> */
static Pair* p_declaration() {
	Pair* out;
	TokenList* save = token;
	
	token = save;
	if ((out = p_var_declaration())) return out;
	token = save;
	if ((out = p_fun_declaration())) return out;
	return NULL;
}

/* <declaration-list> ::= <declaration-list> <declaration> | <declaration> */
static Pair* p_declaration_list() {

	Pair* car;
	Pair* start = NULL;
	Pair* cur   = NULL;

	while (token && (car = p_declaration())) {
		if (start) {
			cur->cdr = Pair_new(NULL, car, NULL);
			cur      = cur->cdr;
		} else {
			start    = Pair_new(NULL, car, NULL);
			cur      = start;
		}
	}

	/* The entire body consists of productions so it should
	   be impossible for there to be remaining tokens here.

	   In fact, this is where it should exit if there are
	   any unparsable token sequences */
	if (token) {
		Pair_free(start);
		return NULL;
	}

	return start;
}

/* <program> ::= <declaration-list> */
static Pair* p_program(void) {
	Pair*  cdr = p_declaration_list();
	return cdr ? Pair_new("program", NULL, cdr) : NULL;
}

Pair* parse(TokenList* tokens) {
	token = tokens;
	// look(0);
	return p_program();
}

