
#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>

#include "pair.h"
#include "type.h"

void codegen(FILE* file, Pair* ast);

#endif