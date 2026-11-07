#include "lexer_identifier.h"

#include <stdbool.h>
#include <string.h>

TokenType lexer_identify_word(
    const char *start,
    size_t length
) {
    if (length == strlen("ص") &&
        memcmp(start, "ص", length) == 0)
        return TOKEN_INT_TYPE;

    if (length == strlen("شروع") &&
        memcmp(start, "شروع", length) == 0)
        return TOKEN_MAIN;

    if (length == strlen("چاپ_ف") &&
        memcmp(start, "چاپ_ف", length) == 0)
        return TOKEN_PRINTF;

    return TOKEN_UNKNOWN;
}

static bool is_word_boundary(Lexer *lexer)
{
    char c = lexer_current(lexer);

    if (c == '\0' ||
        c == ' ' || c == '\t' ||
        c == '\r' || c == '\n' ||
        c == '(' || c == ')' ||
        c == '{' || c == '}' ||
        c == '"' || c == ';')
        return true;

    if (c == '-' && lexer_peek(lexer) == '>')
        return true;

    if ((unsigned char)c == 0xD8 &&
        (unsigned char)lexer_peek(lexer) == 0x9B)
        return true;

    return false;
}

Token lexer_read_identifier(
    Lexer *lexer,
    size_t start,
    size_t line,
    size_t column
) {
    while (!is_word_boundary(lexer))
        lexer_advance(lexer);

    TokenType type = lexer_identify_word(
        lexer->source + start,
        lexer->position - start
    );

    return lexer_make_token(
        lexer,
        type,
        start,
        line,
        column
    );
}