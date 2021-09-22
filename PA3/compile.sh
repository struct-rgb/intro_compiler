#!/bin/sh
src='src/main.c src/lexer.c src/str.c src/parser.c src/pair.c src/idtable.c src/symboltable.c src/type.c src/semantics.c'
flags='-std=c11 -Wall -Werror'
gcc -o semantics $src $flags 
