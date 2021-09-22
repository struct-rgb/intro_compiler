
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


typedef struct Strbuf {

    /* current length of the string */
    size_t length;

    /* current size of the string buffer */
    size_t capacity;

    /* the NUL terminated backing buffer for the string */
    char*  buffer;

} Strbuf;


Strbuf* Strbuf_new(size_t capacity) {

    Strbuf* s = malloc(sizeof(Strbuf));
    if (!s) goto fail_1;

    s->buffer = calloc(sizeof(char), capacity);
    if (!s->buffer) goto fail_2;

    s->length   = 0;
    s->capacity = capacity;

    return s;

fail_2:
    free(s);
fail_1:
    return NULL;
}

void Strbuf_free(Strbuf* s) {
    free(s->buffer);
    free(s);
}

void Strbuf_clear(Strbuf* s) {
    s->length = 0;
}

// void Strbuf_append_nulterm(Strbuf* s, char* nulterm) {
    
//     size_t nullen = strlen(nulterm);
//     size_t sum    = s->length + nullen;

//     if (s->capacity - 1 < sum) {
//         s->buffer   = realloc(s->buffer, sum + 1);
//         s->capacity = sum + 1;
//     }

//     for (size_t i = s->length; i < nullen; ++i) {
//         s->buffer[i] = nulterm[i];
//     }
//     s->buffer[sum + 1] = '\0';

// }

void Strbuf_pushc(Strbuf* s, char glyph) {

    if (s->length + 1 == s->capacity) {
        s->capacity *= 2;
        s->buffer = realloc(s->buffer, s->capacity);
    }

    s->buffer[s->length] = glyph;

    /* prevent null terminators from counting towards the length */
    if (glyph) ++s->length;

}

void Strbuf_copy_nulterm(Strbuf* s, char* nulterm) {
    
    size_t nullen = strlen(nulterm);

    if (s->capacity < nullen + 1) {
        s->buffer   = realloc(s->buffer, nullen + 1);
        s->capacity = nullen + 1;
    }

    for (size_t i = 0; i < nullen; ++i) {
        s->buffer[i] = nulterm[i];
    }
    s->buffer[nullen] = '\0';
    s->length         = nullen;

}

typedef struct Token {
    
    /* line number token was found on */
    int          lineno;

    /* the type of the discovered token */
    char const*  symbol;

    /* a string buffer for the lexeme */
    Strbuf*      lexeme;

} Token;

Token* Token_new(void) {

    Token* t = malloc(sizeof(Token));
    if (!t) goto fail_1;

    t->lexeme = Strbuf_new(32);
    if (!t->lexeme) goto fail_2;

    t->lineno = 1;
    t->symbol = NULL;

    return t;

fail_2:
    free(t);
fail_1:
    return NULL;
}

void Token_free(Token* token) {
    Strbuf_free(token->lexeme);
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
bool is_letter(char glyph) {
    return ('a' <= glyph && glyph <= 'z') || ('A' <= glyph && glyph <= 'Z');
}

/* determine whether a char matches [0-9] */
bool is_digit(char glyph) {
    return '0' <= glyph && glyph <= '9';
}

/* determine whether a string is one of ("if", "int", "void", "else", "while", "return") */
bool is_keyword(Strbuf* lexeme) {

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

        Strbuf_clear(token->lexeme);
        while (glyph != EOF && (is_letter(glyph) || is_digit(glyph))) {
            Strbuf_pushc(token->lexeme, glyph);
            glyph = fgetc(source);
        }
        Strbuf_pushc(token->lexeme, '\0');

        token->symbol = is_keyword(token->lexeme)
            ? TOKEN_TYPE_KEY
            : TOKEN_TYPE_ID;

        ungetc(glyph, source);

        return true;
    }

    if (is_digit(glyph)) {

        Strbuf_clear(token->lexeme);
        while (glyph != EOF && is_digit(glyph)) {
            Strbuf_pushc(token->lexeme, glyph);
            glyph = fgetc(source);
        }
        Strbuf_pushc(token->lexeme, '\0');

        ungetc(glyph, source);

        token->symbol = TOKEN_TYPE_NUM;
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
            token->symbol    = TOKEN_TYPE_SYM;
            Strbuf_clear(token->lexeme);
            Strbuf_pushc(token->lexeme, glyph);
            Strbuf_pushc(token->lexeme, '\0');
            return true;

        case '/':
            if ((glyph = fgetc(source)) == '*') {
                /* this begins a commment, remeber the line in case of error */
                int error_line = token->lineno;
            comment:
                if ((glyph = fgetc(source)) == EOF) {
                    /* the comment runs to EOF, this is an error */
                    token->symbol = TOKEN_TYPE_ERROR;
                    Strbuf_copy_nulterm(token->lexeme, "/*");
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
                token->symbol = TOKEN_TYPE_SYM;
                Strbuf_copy_nulterm(token->lexeme, "/");
                ungetc(glyph, source);
            }
            return true;
        
        /* these can all have an optional trailing '=' */
        case '<':
            token->symbol = TOKEN_TYPE_SYM;
            if ((glyph = fgetc(source)) == '=') {
                Strbuf_copy_nulterm(token->lexeme, "<=");
            } else {
                Strbuf_copy_nulterm(token->lexeme, "<");
                ungetc(glyph, source);
            }
            return true;

        case '>':
            token->symbol = TOKEN_TYPE_SYM;
            if ((glyph = fgetc(source)) == '=') {
                Strbuf_copy_nulterm(token->lexeme, ">=");
            } else {
                Strbuf_copy_nulterm(token->lexeme, ">");
                ungetc(glyph, source);
            }
            return true;

        case '=':
            token->symbol = TOKEN_TYPE_SYM;
            if ((glyph = fgetc(source)) == '=') {
                Strbuf_copy_nulterm(token->lexeme, "==");
            } else {
                Strbuf_copy_nulterm(token->lexeme, "=");
                ungetc(glyph, source);
            }
            return true;

        /* '!' only occurs in the digraph "!=" */
        case '!':
            if (fgetc(source) == '=') {
                token->symbol = TOKEN_TYPE_SYM;
                Strbuf_copy_nulterm(token->lexeme, "!=");
            } else {
                token->symbol = TOKEN_TYPE_ERROR;
                Strbuf_copy_nulterm(token->lexeme, "!");
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
            token->symbol = TOKEN_TYPE_ERROR;
            Strbuf_clear(token->lexeme);
            Strbuf_pushc(token->lexeme, glyph);
            Strbuf_pushc(token->lexeme, '\0');
            return true;
    }

    return false;
}

int main(int argc, char** argv) {

    if (argc != 3) {
        fprintf(stderr, "usage: ./lexer <input file> <output file>\n");
        exit(1);
    }

    /* THE TOKEN STATE SHOULD BE TREATED AS READONLY AFTER LEX CALL */
    Token* token = Token_new();
    if (!token) exit(1);
    
    FILE* source = fopen(argv[1], "r");
    if (!source) {
        fprintf(stderr, "Was unable to open input file %s\n", argv[1]);
        exit(1);
    }

    FILE* sink = fopen(argv[2], "w");
    if (!sink) {
        fprintf(stderr, "Was unable to open output file %s\n", argv[2]);
        exit(1);
    }

    while (lex(source, token)) {
        fprintf(sink, "(%d,%s,\"%s\")\n", token->lineno, token->symbol, token->lexeme->buffer);
        if (token->symbol == TOKEN_TYPE_ERROR) break;
    }

    Token_free(token);
    fclose(source);
    fclose(sink);

    return 0;
}
