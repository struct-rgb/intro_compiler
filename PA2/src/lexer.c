
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../include/str.h"
#include "../include/lexer.h"

Token* Token_new(void) {

    Token* t = malloc(sizeof(Token));
    if (!t) goto fail_1;

    t->lexeme = Str_new(32);
    if (!t->lexeme) goto fail_2;

    t->lineno = 1;
    t->symbol = TokenType_ERR;

    return t;

fail_2:
    free(t);
fail_1:
    return NULL;
}

void Token_free(Token* token) {
    Str_free(token->lexeme);
    free(token);
}

char const KEYWORD_IF[]     = "if";
char const KEYWORD_INT[]    = "int";
char const KEYWORD_VOID[]   = "void";
char const KEYWORD_ELSE[]   = "else";
char const KEYWORD_WHILE[]  = "while";
char const KEYWORD_RETURN[] = "return";

char const* const KEYWORDS[] = {
    KEYWORD_IF, KEYWORD_INT, KEYWORD_VOID, KEYWORD_ELSE, KEYWORD_WHILE, KEYWORD_RETURN
};

/* determine whether a char matches [a-zA-Z] */
static bool is_letter(char glyph) {
    return ('a' <= glyph && glyph <= 'z') || ('A' <= glyph && glyph <= 'Z');
}

/* determine whether a char matches [0-9] */
static bool is_digit(char glyph) {
    return '0' <= glyph && glyph <= '9';
}

/* determine whether a string is one of ("if", "int", "void", "else", "while", "return") */
static bool is_keyword(Str* lexeme) {

    size_t const length = lexeme->length;
    
    /* here's a fast heuristic */
    if (!(2 <= length && length <= 6))
        return false;
    
    /* brute force approach */
    for (int i = 0; i < 6; ++i) {
        if (strcmp(lexeme->buffer, KEYWORDS[i]) == 0)
            return true;
    }

    /* that failed */
    return false;
}

char const TOKEN_TYPE_ID[]    = "ID";
char const TOKEN_TYPE_SYM[]   = "SYM";
char const TOKEN_TYPE_KEY[]   = "KEY";
char const TOKEN_TYPE_NUM[]   = "NUM";
char const TOKEN_TYPE_ERROR[] = "ERROR";

char const* TokenType_to_string(TokenType type) {
    switch(type) {
        case TokenType_ID:  return TOKEN_TYPE_ID;
        case TokenType_SYM: return TOKEN_TYPE_SYM;
        case TokenType_KEY: return TOKEN_TYPE_KEY;
        case TokenType_NUM: return TOKEN_TYPE_NUM;
        case TokenType_ERR: return TOKEN_TYPE_ERROR;
        default:
            return TOKEN_TYPE_ERROR;
    }
}

/*
 * Reads in a token from the stream if possible and returns
 * true, otherwisevreturns false.
 * 
 * THE TOKEN STATE SHOULD BE TREATED AS READONLY BY CALLER
 */
bool lex(FILE* source, struct Token* token) {
    
    int glyph;

start:
    if ((glyph = fgetc(source)) == EOF)
        return false;

    if (is_letter(glyph)) {

        Str_clear(token->lexeme);
        while (glyph != EOF && (is_letter(glyph) || is_digit(glyph))) {
            Str_pushchar(token->lexeme, glyph);
            glyph = fgetc(source);
        }
        Str_pushchar(token->lexeme, '\0');

        token->symbol = is_keyword(token->lexeme)
            ? TokenType_KEY
            : TokenType_ID;

        ungetc(glyph, source);

        return true;
    }

    if (is_digit(glyph)) {

        Str_clear(token->lexeme);
        while (glyph != EOF && is_digit(glyph)) {
            Str_pushchar(token->lexeme, glyph);
            glyph = fgetc(source);
        }
        Str_pushchar(token->lexeme, '\0');

        ungetc(glyph, source);

        token->symbol = TokenType_NUM;
        return true;
    }

    switch (glyph) {

        /* all of these occur alone */
        case '+':

        case '-':

        case '*':

        case ';':

        case ',':

        case '(':

        case ')':

        case '[':

        case ']':

        case '{':

        case '}':
            token->symbol    = TokenType_SYM;
            Str_clear(token->lexeme);
            Str_pushchar(token->lexeme, glyph);
            Str_pushchar(token->lexeme, '\0');
            return true;

        case '/':
            if ((glyph = fgetc(source)) == '*') {
                /* this begins a commment, remeber the line in case of error */
                int error_line = token->lineno;
            comment:
                if ((glyph = fgetc(source)) == EOF) {
                    /* the comment runs to EOF, this is an error */
                    token->symbol = TokenType_ERR;
                    Str_copy_nulterm(token->lexeme, "/*");
                    token->lineno = error_line;
                    return true;
                }

                switch (glyph) {
                    case '*':
                        /* check to see if this is the end */
                        if ((glyph = fgetc(source)) == '/') {
                            /* return to normal lexing */
                            goto start;
                        }
                        /* put it back and keep consuming comment */
                        ungetc(glyph, source);
                        goto comment;

                    /* we still need to count lines */
                    case '\n':
                        token->lineno++;
                    /* fallthrough */
                    /* ignore the character and continue */
                    default:
                        goto comment;
                }
                /* never exits through this path */
            } else {
                token->symbol = TokenType_SYM;
                Str_copy_nulterm(token->lexeme, "/");
                ungetc(glyph, source);
            }
            return true;
        
        /* these can all have an optional trailing '=' */
        case '<':
            token->symbol = TokenType_SYM;
            if ((glyph = fgetc(source)) == '=') {
                Str_copy_nulterm(token->lexeme, "<=");
            } else {
                Str_copy_nulterm(token->lexeme, "<");
                ungetc(glyph, source);
            }
            return true;

        case '>':
            token->symbol = TokenType_SYM;
            if ((glyph = fgetc(source)) == '=') {
                Str_copy_nulterm(token->lexeme, ">=");
            } else {
                Str_copy_nulterm(token->lexeme, ">");
                ungetc(glyph, source);
            }
            return true;

        case '=':
            token->symbol = TokenType_SYM;
            if ((glyph = fgetc(source)) == '=') {
                Str_copy_nulterm(token->lexeme, "==");
            } else {
                Str_copy_nulterm(token->lexeme, "=");
                ungetc(glyph, source);
            }
            return true;

        /* '!' only occurs in the digraph "!=" */
        case '!':
            if (fgetc(source) == '=') {
                token->symbol = TokenType_SYM;
                Str_copy_nulterm(token->lexeme, "!=");
            } else {
                token->symbol = TokenType_ERR;
                Str_copy_nulterm(token->lexeme, "!");
            }
            return true;

        /* whitespace, but we need to count lines */
        case '\n':
            token->lineno++;
        /* whitespace, ignore */
        case ' ':
        /* whitespace, ignore */
        case '\t':
            goto start;

        default:
            token->symbol = TokenType_ERR;
            Str_clear(token->lexeme);
            Str_pushchar(token->lexeme, glyph);
            Str_pushchar(token->lexeme, '\0');
            return true;
    }

    return false;
}

void lexfree(TokenList* tokens) {
    for (TokenList* t = tokens,* tmp; t; t = tmp) {
        tmp = t->next;
        Str_free(t->value.lexeme);
        free(t);
    }
}

TokenList* lexlist(FILE* source) {

    /* THE TOKEN STATE SHOULD BE TREATED AS READONLY AFTER LEX CALL */
    Token* token = Token_new();
    if (!token) goto fail_1;

    TokenList* start = NULL;
    TokenList* node  = NULL;

    while (lex(source, token)) {

        if (start) {
            node->next = malloc(sizeof (TokenList));
            if (!node->next) goto fail_3;
            node = node->next;
            node->next = NULL;
        } else {
            start = malloc(sizeof (TokenList));
            if (!start) goto fail_2;
            start->next = NULL;
            node = start;
        }

        node->value        = *token;
        node->value.lexeme = Str_dup(token->lexeme);
    }

    return start;

fail_3:
    for (TokenList* t = start,* tmp; t; t = tmp) {
        tmp = t->next;
        Str_free(t->value.lexeme);
        free(t);
    }
fail_2:
    free(token);
fail_1:
    return NULL;
}