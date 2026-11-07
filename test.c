#include <stdio.h>
#include "lexer/lexer.h"

int main(void)
{
    const char *source =
        "ص -> شروع() {\n"
        "    چاپ_ف(\"سلام\")؛\n"
        "}";

    Lexer lexer;
    lexer_init(&lexer, source);

    Token token;

    do {
        token = lexer_next(&lexer);

        printf("%-12s | %.*s | line: %zu\n",
               token_type_name(token.type),
               (int)token.length,
               token.start,
               token.line);

    } while (token.type != TOKEN_EOF);

    return 0;
}