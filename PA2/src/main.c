
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "../include/pair.h"
#include "../include/lexer.h"
#include "../include/parser.h"

void segfault_handler(int signal) {
    exit(2);
}

int main(int argc, char** argv) {

    signal(SIGSEGV, segfault_handler);

    if (argc != 3) {
        fprintf(stderr, "usage: ./parser <input file> <output file>\n");
        exit(1);
    }
    
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

    TokenList* tokens = lexlist(source);
    if (!tokens) exit(1);

    Pair* ast = parse(tokens);

  	write_ast(sink, ast);

  	Pair_free(ast);
  	lexfree(tokens);

    fclose(source);
    fclose(sink);

    return 0;
}
