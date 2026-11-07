#ifndef LEXER_INTERNAL_H
#define LEXER_INTERNAL_H

#include "../lexer.h"

char lexer_current(Lexer *lexer);
char lexer_peek(Lexer *lexer);
void lexer_advance(Lexer *lexer);
void lexer_skip_whitespace(Lexer *lexer);

Token lexer_make_token(
    Lexer *lexer,
    TokenType type,
    size_t start,
    size_t line,
    size_t column
);

TokenType lexer_identify_word(
    const char *start,
    size_t length
);

Token lexer_read_string(
    Lexer *lexer,
    size_t start,
    size_t line,
    size_t column
);

Token lexer_read_identifier(
    Lexer *lexer,
    size_t start,
    size_t line,
    size_t column
);

#endif