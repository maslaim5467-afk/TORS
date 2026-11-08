
#include "code_generator.h"

#include <string.h>

static bool set_error(
    char *error,
    size_t error_size,
    const char *message
)
{
    if (error && error_size > 0)
        snprintf(error, error_size, "%s", message);

    return false;
}

static bool emit_string(
    FILE *output,
    const Token *token,
    size_t index
)
{
    if (!token || !token->start || token->length == 0)
        return false;

    const char *start = token->start;
    size_t length = token->length;

    if (length >= 2 &&
        start[0] == '"' &&
        start[length - 1] == '"') {
        start++;
        length -= 2;
    }

    if (fprintf(
            output,
            "@.str%zu = private unnamed_addr constant [%zu x i8] c\"",
            index,
            length + 1
        ) < 0)
        return false;

    for (size_t i = 0; i < length; i++) {
        unsigned char byte = (unsigned char)start[i];

        if (fprintf(output, "\\%02X", byte) < 0)
            return false;
    }

    return fputs("\\00\", align 1\n", output) != EOF;
}

bool codegen_generate(
    const ASTProgram *program,
    FILE *output,
    char *error,
    size_t error_size
)
{
    if (!program || !output)
        return set_error(error, error_size,
                         "Invalid code generator arguments");

    const ASTBlock *body = &program->main_function.body;

    size_t print_count = 0;

    for (size_t i = 0; i < body->count; i++) {
        const ASTNode *node = &body->statements[i];

        if (node->type == AST_PRINT_STATEMENT)
            print_count++;
        else if (node->type != AST_VARIABLE_DECLARATION)
            return set_error(error, error_size,
                             "Unsupported AST node");
    }

    if (fputs("declare i32 @printf(ptr, ...)\n\n", output) == EOF)
        return set_error(error, error_size, "Failed to write LLVM IR");

    size_t string_index = 0;

    for (size_t i = 0; i < body->count; i++) {
        if (body->statements[i].type == AST_PRINT_STATEMENT) {
            if (!emit_string(
                    output,
                    &body->statements[i].value,
                    string_index++))
                return set_error(error, error_size,
                                 "Failed to generate string constant");
        }
    }

    if (print_count > 0 && fputc('\n', output) == EOF)
        return set_error(error, error_size, "Failed to write LLVM IR");

    if (fputs(
            "define i32 @main() {\n"
            "entry:\n",
            output
        ) == EOF)
        return set_error(error, error_size, "Failed to write main function");

    size_t variable_index = 0;

    /* Allocate local variables */
    for (size_t i = 0; i < body->count; i++) {
        if (body->statements[i].type == AST_VARIABLE_DECLARATION) {
            if (fprintf(
                    output,
                    "    %%var%zu = alloca i32, align 4\n",
                    variable_index++
                ) < 0)
                return set_error(error, error_size,
                                 "Failed to allocate variable");
        }
    }

    variable_index = 0;
    string_index = 0;

    /* Emit statements */
    for (size_t i = 0; i < body->count; i++) {
        const ASTNode *node = &body->statements[i];

        if (node->type == AST_VARIABLE_DECLARATION) {
            char number[32];

            if (node->initializer.length >= sizeof(number))
                return set_error(error, error_size,
                                 "Integer literal is too long");

            memcpy(number, node->initializer.start,
                   node->initializer.length);
            number[node->initializer.length] = '\0';

            if (fprintf(
                    output,
                    "    store i32 %s, ptr %%var%zu, align 4\n",
                    number,
                    variable_index++
                ) < 0)
                return set_error(error, error_size,
                                 "Failed to initialize variable");
        }
        else if (node->type == AST_PRINT_STATEMENT) {
            if (fprintf(
                    output,
                    "    call i32 (ptr, ...) @printf(ptr @.str%zu)\n",
                    string_index++
                ) < 0)
                return set_error(error, error_size,
                                 "Failed to write print call");
        }
    }

    if (fputs(
            "    ret i32 0\n"
            "}\n",
            output
        ) == EOF)
        return set_error(error, error_size,
                         "Failed to finish main function");

    if (ferror(output))
        return set_error(error, error_size, "Output stream error");

    return true;
}
