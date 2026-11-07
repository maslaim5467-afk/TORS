#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stddef.h>

#include "../lexer/lexer.h"

typedef enum {
    AST_PRINT_STATEMENT
} ASTNodeType;

typedef struct {
    ASTNodeType type;
    Token value;
} ASTNode;

typedef struct {
    ASTNode *statements;
    size_t count;
    size_t capacity;
} ASTBlock;

typedef struct {
    Token return_type;
    Token name;
    ASTBlock body;
} ASTFunction;

typedef struct {
    ASTFunction main_function;
} ASTProgram;

typedef struct {
    Lexer *lexer;
    Token current;
    bool has_error;
    char error[256];
    size_t error_line;
    size_t error_column;
} Parser;

void parser_init(Parser *parser, Lexer *lexer);

bool parser_parse(
    Parser *parser,
    ASTProgram *program
);

void ast_free(ASTProgram *program);

#endif