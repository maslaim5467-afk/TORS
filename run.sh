#!/data/data/com.termux/files/usr/bin/bash

set -eu

if [ "$#" -ne 1 ]; then
    echo "Usage: ./run.sh <source.T>"
    exit 1
fi

echo "Building Persian compiler..."

clang -std=c11 -Wall -Wextra -Wpedantic -I. \
    lexer/*.c \
    lexer/lexer1/*.c \
    parser/*.c \
    code_generator/*.c \
    main.c \
    -o compiler

echo "Running compiler..."

./compiler "$1"