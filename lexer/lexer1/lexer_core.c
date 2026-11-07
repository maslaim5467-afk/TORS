#include "lexer_core.h"

char lexer_current(Lexer *lexer)
{
    return lexer->source[lexer->position];
}

char lexer_peek(Lexer *lexer)
{
    if (lexer_current(lexer) == '\0')
        return '\0';

    return lexer->source[lexer->position + 1];
}

void lexer_advance(Lexer *lexer)
{
    if (lexer_current(lexer) == '\0')
        return;

    if (lexer_current(lexer) == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }

    lexer->position++;
}

void lexer_skip_whitespace(Lexer *lexer)
{
    while (lexer_current(lexer) == ' ' ||
           lexer_current(lexer) == '\t' ||
           lexer_current(lexer) == '\r' ||
           lexer_current(lexer) == '\n') {
        lexer_advance(lexer);
    }
}

Token lexer_make_token(
    Lexer *lexer,
    TokenType type,
    size_t start,
    size_t line,
    size_t column
) {
    Token token = {
        .type = type,
        .start = lexer->source + start,
        .length = lexer->position - start,
        .line = line,
        .column = column
    };

    return token;
}