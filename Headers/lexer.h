#pragma once

// Numbers
typedef enum {
    TOKEN_NUMBER,
    TOKEN_PLUS, TOKEN_MINUS, TOKEN_MUL, TOKEN_DIV,
    TOKEN_EQUALS,
    TOKEN_IDENTIFIER, TOKEN_TERM, TOKEN_DROP,
    TOKEN_TYPE,
    TOKEN_EOF
} TokenType;

// Types
typedef enum {
    SORRY_UNKNOWN = 0, // infer from value (default via calloc)
    SORRY_I64,
    SORRY_BOOL,
    SORRY_STR,
} SorryType;

typedef struct {
    TokenType type;
    long value; // long to match LLVM's int64
    char name[32]; // TODO: Use a more efficient data structure for identifiers
    SorryType val_type; // For TOKEN_TYPE tokens
} Token;

Token get_next_token(const char **input);

// AST stuff
typedef enum {
    NODE_NUMBER,
    NODE_BINOP, // Binary operation node
    NODE_TYPE,
    NODE_LINE,
    NODE_COLUMN,
    NODE_ASSIGN,
    NODE_IDENTIFIER,
    NODE_SEQUENCE, // Chain of statements; left runs first, right is returned
} NodeType;

typedef struct ASTNode {
    NodeType type;
    SorryType val_type; // Type annotation (SORRY_UNKNOWN = infer)
    long value; // For numbers
    char name[32]; // For variables
    int op; // Operator, "+" or "-"
    struct ASTNode *left, *right;
} ASTNode;