#pragma once

typedef enum {
    TOKEN_NUMBER,
    TOKEN_PLUS, TOKEN_MINUS, TOKEN_MUL, TOKEN_DIV,
    TOKEN_EQUALS,
    TOKEN_IDENTIFIER, TOKEN_SEMICOLON,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    int value; // This should be int, frick you autocomplete
    char name[32]; // TODO: Use a more efficient data structure for identifiers
} Token;

Token get_next_token(const char **input);