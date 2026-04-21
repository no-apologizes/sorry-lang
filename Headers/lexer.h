#pragma once
#include <stdbool.h>

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
    // No inference, explicit typing
    SORRY_UNKNOWN,
    SORRY_I64,  // 64-bit integer
    SORRY_F32,  // 32-bit float, ~7 decimal digits
    SORRY_F64,  // 64-bit float, ~15 decimal digits
    SORRY_BOOL, // i1, boolean
    SORRY_STR,  // i8*, string pointer
} SorryType;

typedef struct {
    TokenType type;
    long value;         // long to match LLVM's int64
    double float_value; // For floating-point literals
    bool is_float;       // 1 if the literal contained a '.'
    char name[32];      // TODO: Use a more efficient data structure for identifiers
    SorryType val_type; // For TOKEN_TYPE tokens
} Token;

Token get_next_token(const char **input);

// AST stuff
typedef enum {
    NODE_NUMBER,     // Use you're brain
    NODE_BINOP,      // Binary operation node
    NODE_TYPE,       // Type of... thing, etc., str, long, int, longlong
    NODE_LINE,       // For error feedback
    NODE_COLUMN,     // For error feedback
    NODE_ASSIGN,     // Node
    NODE_IDENTIFIER, // For variables
    NODE_SEQUENCE,   // Chain of statements; left runs first, right is returned
} NodeType;

typedef struct ASTNode {
    NodeType type;
    SorryType val_type; // Type annotation (SORRY_UNKNOWN = infer)
    long value;         // For numbers
    double float_value; // For floating point literals
    int is_float;       // 1 if this number literal has a decimal point
    char name[32];      // For variables
    int op;             // Operator, "+" or "-"
    struct ASTNode *left, *right;
} ASTNode;