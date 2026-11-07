#include <stdio.h>

#include "lexer/lexer.h"
#include "parser/parser.h"

int main(void)
{
    const char *source =
        "ص -> شروع() {\n"
        "    چاپ_ف(\"سلام\")؛\n"
        "}";

    Lexer lexer;
    Parser parser;
    ASTProgram program = {0};

    lexer_init(&lexer, source);
    parser_init(&parser, &lexer);

    if (!parser_parse(&parser, &program)) {
        fprintf(
            stderr,
            "Parser Error at %zu:%zu: %s\n",
            parser.error_line,
            parser.error_column,
            parser.error
        );

        return 1;
    }

    printf("Parsing successful!\n");
    printf(
        "Statements: %zu\n",
        program.main_function.body.count
    );

    ast_free(&program);

    return 0;
}