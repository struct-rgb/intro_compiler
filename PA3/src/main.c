
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "../include/pair.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/semantics.h"

void segfault_handler(int signal) {
    fprintf(stderr, "(segmentation fault)\n");
    exit(2);
}

int main(int argc, char** argv) {

    signal(SIGSEGV, segfault_handler);

    if (argc != 3) {
        fprintf(stderr, "usage: ./semantics <input file> <output file>\n");
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
    if (!ast) goto fail;

    Semantic s;
    if ((s = check_semantics(ast)) == Semantic_OK) {
        write_ast(sink, ast);
    } else {
        fprintf(stderr, "Failed semantic analysis: %d\n", s);
    }

fail:
	Pair_free(ast);
	lexfree(tokens);

    fclose(source);
    fclose(sink);

    return 0;
}
