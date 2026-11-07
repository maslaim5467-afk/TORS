#include <stdio.h>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "code_generator/code_generator.h"

int main(void) {
    const char *source =
        "ص -> شروع() {\n"
        "    چاپ_ف(\"سلام\")؛\n"
        "}\n";

    Lexer lexer;
    Parser parser;
    ASTProgram program = {0};

    lexer_init(&lexer, source);
    parser_init(&parser, &lexer);

    if (!parser_parse(&parser, &program)) {
        fprintf(stderr, "Parser error: %s\n", parser.error);
        return 1;
    }

    FILE *output = fopen("output.ll", "w");

    if (!output) {
        perror("output.ll");
        ast_free(&program);
        return 1;
    }

    char error[256] = {0};

    bool success = codegen_generate(
        &program,
        output,
        error,
        sizeof(error)
    );

    fclose(output);
    ast_free(&program);

    if (!success) {
        fprintf(stderr, "Code Generator error: %s\n", error);
        return 1;
    }

    puts("LLVM IR generated successfully.");

    return 0;
}