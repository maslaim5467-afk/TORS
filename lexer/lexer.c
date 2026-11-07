#include "lexer.h"

#include "lexer1/lexer_core.h"
#include "lexer1/lexer_identifier.h"
#include "lexer1/lexer_string.h"

void lexer_init(Lexer *lexer, const char *source)
{
    lexer->source = source;
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
}

Token lexer_next(Lexer *lexer)
{
    lexer_skip_whitespace(lexer);

    size_t start = lexer->position;
    size_t line = lexer->line;
    size_t column = lexer->column;

    char c = lexer_current(lexer);

    if (c == '\0') {
        return lexer_make_token(
            lexer, TOKEN_EOF, start, line, column
        );
    }

    if (c == '-' && lexer_peek(lexer) == '>') {
        lexer_advance(lexer);
        lexer_advance(lexer);

        return lexer_make_token(
            lexer, TOKEN_ARROW, start, line, column
        );
    }

    if (c == '{' || c == '}' ||
        c == '(' || c == ')') {

        TokenType type;

        switch (c) {
            case '{': type = TOKEN_LBRACE; break;
            case '}': type = TOKEN_RBRACE; break;
            case '(': type = TOKEN_LPAREN; break;
            default:  type = TOKEN_RPAREN; break;
        }

        lexer_advance(lexer);

        return lexer_make_token(
            lexer, type, start, line, column
        );
    }

    /* Persian semicolon: ؛ (UTF-8: D8 9B) */
    if ((unsigned char)c == 0xD8 &&
        (unsigned char)lexer_peek(lexer) == 0x9B) {

        lexer_advance(lexer);
        lexer_advance(lexer);

        return lexer_make_token(
            lexer, TOKEN_SEMICOLON, start, line, column
        );
    }

    if (c == '"') {
        return lexer_read_string(
            lexer, start, line, column
        );
    }

    return lexer_read_identifier(
        lexer, start, line, column
    );
}

const char *token_type_name(TokenType type)
{
    switch (type) {
        case TOKEN_INT_TYPE: return "INT_TYPE";
        case TOKEN_ARROW: return "ARROW";
        case TOKEN_MAIN: return "MAIN";
        case TOKEN_PRINTF: return "PRINTF";
        case TOKEN_STRING: return "STRING";
        case TOKEN_LBRACE: return "LBRACE";
        case TOKEN_RBRACE: return "RBRACE";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_EOF: return "EOF";
        case TOKEN_UNKNOWN: return "UNKNOWN";
    }

    return "INVALID";
}