
#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

#include "pair.h"
#include "lexer.h"

Pair* parse(TokenList* tokens);

void write_ast(FILE* file, Pair* root);

#endif