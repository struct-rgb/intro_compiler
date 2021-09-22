
#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdbool.h>

#include "str.h"

typedef enum TokenType {
	TokenType_ID, TokenType_SYM, TokenType_KEY, TokenType_NUM, TokenType_ERR
} TokenType;

char const* TokenType_to_string(TokenType type);

typedef struct Token {
    
    /* line number token was found on */
    int          lineno;

    /* the type of the discovered token */
    TokenType    symbol;

    /* a string buffer for the lexeme */
    Str*         lexeme;

} Token;

Token* Token_new(void);

void Token_free(Token* token);

bool lex(FILE* source, Token* token);

typedef struct TokenList {
	Token             value;
	struct TokenList* next;
} TokenList;

TokenList* lexlist(FILE* source);

void lexfree(TokenList* tokens);

#endif