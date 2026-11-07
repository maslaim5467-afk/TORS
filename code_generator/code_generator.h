#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "../parser/parser.h"

bool codegen_generate(
    const ASTProgram *program,
    FILE *output,
    char *error,
    size_t error_size
);

#endif