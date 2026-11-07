#include "lexer_string.h"

Token lexer_read_string(
    Lexer *lexer,
    size_t start,
    size_t line,
    size_t column
) {
    lexer_advance(lexer); /* Opening quote */

    while (lexer_current(lexer) != '\0' &&
           lexer_current(lexer) != '"') {

        if (lexer_current(lexer) == '\\') {
            lexer_advance(lexer);

            if (lexer_current(lexer) == '\0')
                break;
        }

        lexer_advance(lexer);
    }

    if (lexer_current(lexer) != '"') {
        return lexer_make_token(
            lexer,
            TOKEN_UNKNOWN,
            start,
            line,
            column
        );
    }

    lexer_advance(lexer); /* Closing quote */

    return lexer_make_token(
        lexer,
        TOKEN_STRING,
        start,
        line,
        column
    );
}