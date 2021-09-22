# Intro to Compilers

A compiler for a subset of the C programming language made for my Intro to Compilers class. Targets MIPS architecture, tests conducted with the spim emulator.

## Subsections

### PA1 (Lexer)

This tokenizes the input.

### PA2 (Parser)

This parses the lexer's output into an abstract syntax tree (AST).

### PA3 (Semantic Analysis)

This does things like typechecking and variable declaration checking on the AST generated in PA2 to ensure that it represents a valid program. Also includes an updated version of the parser.

### PA4 (Code Generation)

This walks the AST and uses it to generated a MIPS assembly file that can then be assembled into a MIPS binary executable.


