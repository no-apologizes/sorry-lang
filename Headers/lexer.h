#pragma once

// Numbers
typedef enum {
    TOKEN_NUMBER,
    TOKEN_PLUS, TOKEN_MINUS, TOKEN_MUL, TOKEN_DIV,
    TOKEN_EQUALS,
    TOKEN_IDENTIFIER, TOKEN_TERM,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    int value; // This should be int, frick you autocomplete
    char name[32]; // TODO: Use a more efficient data structure for identifiers
} Token;

// AST stuff
typedef enum {
    NODE_NUMBER,
    NODE_BINOP, // Binary operation node
    NODE_ASSIGN,
    NODE_IDENTIFIER
} NodeType;

typedef struct ASTNode {
    NodeType type;
    int value; // For numbers
    char name[32]; // For variables
    int op; // Operator, "+" or "-"
    struct ASTNode *left, *right;
} ASTNode;

Token get_next_token(const char **input);