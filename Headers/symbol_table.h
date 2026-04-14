#pragma once
#include <stdbool.h>
#include <llvm-c/Core.h>
#include <string.h>
#include "lexer.h"

// Comments are in the context of my ast
typedef struct {
    char name[32]; // Name of token
    LLVMValueRef ptr;
    SorryType type; // The sorry type of this variable
} Symbol;

typedef struct {
    Symbol* vars; // Yay, dynamic arrays
    int count;
    bool is_constant;
} SymbolTable;

// Helper to find variable
int find_var(const SymbolTable* table, const char* name) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->vars[i].name, name) == 0) return i;
    }
    return -1;
}