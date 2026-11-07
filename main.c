#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "code_generator/code_generator.h"

static char *read_file(const char *path) {
    FILE *file = fopen(path, "rb");

    if (!file) {
        perror(path);
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }

    long size = ftell(file);

    if (size < 0 || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }

    char *buffer = malloc((size_t)size + 1);

    if (!buffer) {
        fclose(file);
        return NULL;
    }

    size_t read_size = fread(buffer, 1, (size_t)size, file);

    if (read_size != (size_t)size || ferror(file)) {
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[read_size] = '\0';
    fclose(file);

    return buffer;
}

static bool has_T_extension(const char *path) {
    size_t length = strlen(path);

    return length >= 2 &&
           path[length - 2] == '.' &&
           path[length - 1] == 'T';
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <source.T>\n", argv[0]);
        return 1;
    }

    const char *source_path = argv[1];

    if (!has_T_extension(source_path)) {
        fprintf(stderr, "Error: source file must have .T extension\n");
        return 1;
    }

    char *source = read_file(source_path);

    if (!source) {
        fprintf(stderr, "Error: cannot read source file\n");
        return 1;
    }

    Lexer lexer;
    Parser parser;
    ASTProgram program = {0};

    lexer_init(&lexer, source);
    parser_init(&parser, &lexer);

    printf("[1/3] Parsing source...\n");

    if (!parser_parse(&parser, &program)) {
        fprintf(
            stderr,
            "Parser error at %zu:%zu: %s\n",
            parser.error_line,
            parser.error_column,
            parser.error
        );

        free(source);
        ast_free(&program);
        return 1;
    }

    size_t base_length = strlen(source_path) - 2;

    char *ir_path = malloc(base_length + 4);
    char *exe_path = malloc(base_length + 1);

    if (!ir_path || !exe_path) {
        fprintf(stderr, "Error: memory allocation failed\n");

        free(ir_path);
        free(exe_path);
        free(source);
        ast_free(&program);
        return 1;
    }

    memcpy(ir_path, source_path, base_length);
    memcpy(exe_path, source_path, base_length);

    strcpy(ir_path + base_length, ".ll");
    exe_path[base_length] = '\0';

    printf("[2/3] Generating LLVM IR...\n");

    FILE *output = fopen(ir_path, "w");

    if (!output) {
        perror(ir_path);

        free(ir_path);
        free(exe_path);
        free(source);
        ast_free(&program);
        return 1;
    }

    char error[256] = {0};

    bool success = codegen_generate(
        &program,
        output,
        error,
        sizeof(error)
    );

    if (fclose(output) != 0) {
        success = false;
    }

    if (!success) {
        fprintf(stderr, "Code Generator error: %s\n", error);

        free(ir_path);
        free(exe_path);
        free(source);
        ast_free(&program);
        return 1;
    }

    printf("[3/3] Generating executable...\n");

    char *command = malloc(strlen(ir_path) + strlen(exe_path) + 32);

    if (!command) {
        fprintf(stderr, "Error: memory allocation failed\n");

        free(ir_path);
        free(exe_path);
        free(source);
        ast_free(&program);
        return 1;
    }

    snprintf(
        command,
        strlen(ir_path) + strlen(exe_path) + 32,
        "clang \"%s\" -o \"%s\"",
        ir_path,
        exe_path
    );

    int result = system(command);

    if (result != 0) {
        fprintf(stderr, "Error: native compilation failed\n");
    } else {
        printf("\nCompilation successful!\n");
        printf("Executable: %s\n", exe_path);
    }

    free(command);
    free(ir_path);
    free(exe_path);
    free(source);
    ast_free(&program);

    return result == 0 ? 0 : 1;
}