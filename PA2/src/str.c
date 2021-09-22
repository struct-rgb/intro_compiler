#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "../include/str.h"

char* strdup(char const* original) {
    char* dup = malloc(strlen(original) + 1);
    if (!dup) return NULL;
    strcpy(dup, original);
    return dup;
}

Str* Str_new(size_t capacity) {

    Str* s = malloc(sizeof(Str));
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

Str* Str_dup(Str* original) {
    Str* s = Str_new(original->capacity);
    if (!s) return NULL;

    Str_copy_nulterm(s, original->buffer);

    return s;
}

void Str_free(Str* s) {
    free(s->buffer);
    free(s);
}

void Str_clear(Str* s) {
    s->length = 0;
}

void Str_pushchar(Str* s, char glyph) {

    if (s->length + 1 == s->capacity) {
        s->capacity *= 2;
        s->buffer = realloc(s->buffer, s->capacity);
    }

    s->buffer[s->length] = glyph;

    /* prevent null terminators from counting towards the length */
    if (glyph) ++s->length;

}

void Str_copy_nulterm(Str* s, char* nulterm) {
    
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