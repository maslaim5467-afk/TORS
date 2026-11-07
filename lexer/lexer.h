#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

typedef enum {
    TOKEN_INT_TYPE,       // ص
    TOKEN_ARROW,          // ->
    TOKEN_MAIN,           // شروع
    TOKEN_PRINTF,         // چاپ_ف
    TOKEN_STRING,         // "سلام"

    TOKEN_LBRACE,         // {
    TOKEN_RBRACE,         // }
    TOKEN_LPAREN,         // (
    TOKEN_RPAREN,         // )
    TOKEN_SEMICOLON,      // ؛

    TOKEN_EOF,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    const char *start;
    size_t length;
    size_t line;
    size_t column;
} Token;

typedef struct {
    const char *source;
    size_t position;
    size_t line;
    size_t column;
} Lexer;

void lexer_init(Lexer *lexer, const char *source);
Token lexer_next(Lexer *lexer);
const char *token_type_name(TokenType type);

#endif