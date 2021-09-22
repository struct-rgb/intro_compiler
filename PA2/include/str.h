
#ifndef STR_H
#define STR_H

typedef struct Str {

    /* current length of the string */
    size_t length;

    /* current size of the string buffer */
    size_t capacity;

    /* the NUL terminated backing buffer for the string */
    char*  buffer;

} Str;

char* strdup(char const* original);

Str* Str_new(size_t capacity);

Str* Str_dup(Str* original);

void Str_free(Str* s);

void Str_clear(Str* s);

void Str_pushchar(Str* s, char glyph);

void Str_copy_nulterm(Str* s, char* nulterm);

#endif