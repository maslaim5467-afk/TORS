#include "code_generator.h"

#include <string.h>

static bool set_error(
    char *error,
    size_t error_size,
    const char *message
) {
    if (error && error_size > 0) {
        snprintf(error, error_size, "%s", message);
    }

    return false;
}

static bool emit_string(
    FILE *output,
    const Token *token,
    size_t index
) {
    if (!token || !token->start || token->length == 0) {
        return false;
    }

    const char *start = token->start;
    size_t length = token->length;

    /*
     * Lexer is expected to include quotation marks
     * in the TOKEN_STRING range.
     */
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
        ) < 0) {
        return false;
    }

    /*
     * Escape every byte.
     * This preserves UTF-8 bytes exactly.
     */
    for (size_t i = 0; i < length; i++) {
        unsigned char byte = (unsigned char)start[i];

        if (fprintf(output, "\\%02X", byte) < 0) {
            return false;
        }
    }

    if (fputs("\\00\", align 1\n", output) == EOF) {
        return false;
    }

    return true;
}

bool codegen_generate(
    const ASTProgram *program,
    FILE *output,
    char *error,
    size_t error_size
) {
    if (!program || !output) {
        return set_error(
            error,
            error_size,
            "Invalid code generator arguments"
        );
    }

    const ASTBlock *body = &program->main_function.body;

    /*
     * Validate AST nodes before generating output.
     */
    for (size_t i = 0; i < body->count; i++) {
        if (body->statements[i].type != AST_PRINT_STATEMENT) {
            return set_error(
                error,
                error_size,
                "Unsupported AST node"
            );
        }
    }

    /*
     * Module declarations.
     */
    if (fputs(
            "declare i32 @printf(ptr, ...)\n\n",
            output
        ) == EOF) {
        return set_error(error, error_size, "Failed to write LLVM IR");
    }

    /*
     * Generate global string constants.
     */
    for (size_t i = 0; i < body->count; i++) {
        if (!emit_string(
                output,
                &body->statements[i].value,
                i
            )) {
            return set_error(
                error,
                error_size,
                "Failed to generate string constant"
            );
        }
    }

    if (body->count > 0 && fputc('\n', output) == EOF) {
        return set_error(error, error_size, "Failed to write LLVM IR");
    }

    /*
     * Generate main function.
     */
    if (fputs(
            "define i32 @main() {\n"
            "entry:\n",
            output
        ) == EOF) {
        return set_error(error, error_size, "Failed to write main function");
    }

    for (size_t i = 0; i < body->count; i++) {
        if (fprintf(
                output,
                "    call i32 (ptr, ...) @printf(ptr @.str%zu)\n",
                i
            ) < 0) {
            return set_error(error, error_size, "Failed to write print call");
        }
    }

    if (fputs(
            "    ret i32 0\n"
            "}\n",
            output
        ) == EOF) {
        return set_error(error, error_size, "Failed to finish main function");
    }

    if (ferror(output)) {
        return set_error(error, error_size, "Output stream error");
    }

    return true;
}