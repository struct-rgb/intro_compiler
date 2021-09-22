#!/bin/sh
gcc -o parser src/main.c src/lexer.c src/str.c src/parser.c src/pair.c -std=c99 -Wall -Werror
