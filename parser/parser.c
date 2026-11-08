
#include "parser.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void advance(Parser *parser)
{
    parser->current = lexer_next(parser->lexer);
}

static bool check(Parser *parser, TokenType type)
{
    return parser->current.type == type;
}

static bool set_error(Parser *parser, const char *message)
{
    if (!parser->has_error) {
        parser->has_error = true;

        snprintf(
            parser->error,
            sizeof(parser->error),
            "%s",
            message
        );

        parser->error_line = parser->current.line;
        parser->error_column = parser->current.column;
    }

    return false;
}

static bool expect(
    Parser *parser,
    TokenType type,
    const char *message
)
{
    if (!check(parser, type))
        return set_error(parser, message);

    advance(parser);
    return true;
}

static bool block_add(ASTBlock *block, ASTNode node)
{
    if (block->count == block->capacity) {
        size_t new_capacity =
            block->capacity == 0 ? 4 : block->capacity * 2;

        if (new_capacity > SIZE_MAX / sizeof(ASTNode))
            return false;

        ASTNode *new_statements = realloc(
            block->statements,
            new_capacity * sizeof(ASTNode)
        );

        if (new_statements == NULL)
            return false;

        block->statements = new_statements;
        block->capacity = new_capacity;
    }

    block->statements[block->count++] = node;
    return true;
}

static bool parse_print(Parser *parser, ASTBlock *block)
{
    ASTNode node = {0};
    node.type = AST_PRINT_STATEMENT;

    advance(parser);

    if (!expect(parser, TOKEN_LPAREN, "Expected '(' after print"))
        return false;

    if (!check(parser, TOKEN_STRING))
        return set_error(parser, "Expected a string inside print");

    node.value = parser->current;
    advance(parser);

    if (!expect(parser, TOKEN_RPAREN, "Expected ')' after string"))
        return false;

    if (!expect(parser, TOKEN_SEMICOLON, "Expected ';' after print"))
        return false;

    if (!block_add(block, node))
        return set_error(parser, "Memory allocation failed");

    return true;
}

static bool token_is_32i(Token token)
{
    return token.length == 3 &&
           memcmp(token.start, "32i", 3) == 0;
}

static bool parse_variable(Parser *parser, ASTBlock *block)
{
    ASTNode node = {0};
    node.type = AST_VARIABLE_DECLARATION;

    /* Consume ص */
    advance(parser);

    if (!expect(parser, TOKEN_ARROW,
                "Expected '->' after variable type"))
        return false;

    if (!check(parser, TOKEN_IDENTIFIER))
        return set_error(parser, "Expected variable name");

    node.name = parser->current;
    advance(parser);

    if (!expect(parser, TOKEN_LPAREN,
                "Expected '(' before data type"))
        return false;

    if (!check(parser, TOKEN_UNSIGNED_INT32))
        return set_error(parser, "Expected integer type 32i");

    node.data_type = parser->current;

    if (!token_is_32i(node.data_type))
        return set_error(parser, "Only 32i is supported currently");

    advance(parser);

    if (!expect(parser, TOKEN_RPAREN,
                "Expected ')' after data type"))
        return false;

    if (!expect(parser, TOKEN_ASSIGN,
                "Expected '=' before initial value"))
        return false;

    if (!check(parser, TOKEN_INTEGER_LITERAL))
        return set_error(parser, "Expected integer initial value");

    node.initializer = parser->current;
    advance(parser);

    if (!expect(parser, TOKEN_SEMICOLON,
                "Expected '؛' after variable declaration"))
        return false;

    if (!block_add(block, node))
        return set_error(parser, "Memory allocation failed");

    return true;
}

static bool parse_statement(Parser *parser, ASTBlock *block)
{
    switch (parser->current.type) {
        case TOKEN_PRINTF:
            return parse_print(parser, block);

        case TOKEN_INT_TYPE:
            return parse_variable(parser, block);

        default:
            return set_error(
                parser,
                "Unexpected token inside function body"
            );
    }
}

static bool parse_block(Parser *parser, ASTBlock *block)
{
    if (!expect(parser, TOKEN_LBRACE,
                "Expected '{' to begin function body"))
        return false;

    while (!check(parser, TOKEN_RBRACE) &&
           !check(parser, TOKEN_EOF)) {
        if (!parse_statement(parser, block))
            return false;
    }

    return expect(parser, TOKEN_RBRACE,
                  "Expected '}' to close function body");
}

static bool parse_function(Parser *parser, ASTFunction *function)
{
    if (!check(parser, TOKEN_INT_TYPE))
        return set_error(parser, "Expected function return type");

    function->return_type = parser->current;
    advance(parser);

    if (!expect(parser, TOKEN_ARROW,
                "Expected '->' after return type"))
        return false;

    if (!check(parser, TOKEN_MAIN))
        return set_error(parser, "Expected main function name");

    function->name = parser->current;
    advance(parser);

    if (!expect(parser, TOKEN_LPAREN,
                "Expected '(' after function name"))
        return false;

    if (!expect(parser, TOKEN_RPAREN,
                "Expected ')' after function parameters"))
        return false;

    return parse_block(parser, &function->body);
}

void parser_init(Parser *parser, Lexer *lexer)
{
    memset(parser, 0, sizeof(*parser));
    parser->lexer = lexer;
    advance(parser);
}

bool parser_parse(Parser *parser, ASTProgram *program)
{
    if (parser == NULL || parser->lexer == NULL || program == NULL)
        return false;

    memset(program, 0, sizeof(*program));

    if (parser->has_error)
        return false;

    if (!parse_function(parser, &program->main_function)) {
        ast_free(program);
        return false;
    }

    if (!expect(parser, TOKEN_EOF,
                "Unexpected content after main function")) {
        ast_free(program);
        return false;
    }

    return true;
}

void ast_free(ASTProgram *program)
{
    if (program == NULL)
        return;

    free(program->main_function.body.statements);
    memset(program, 0, sizeof(*program));
}
