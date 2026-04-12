#pragma once
#include <llvm-c/Core.h>
#include <string.h>

// Comments are in the context of my ast
typedef struct {
    char name[32]; // Name of token
    LLVMValueRef ptr;
} Symbol;

typedef struct {
    Symbol vars[100]; // Hardcoded max of 100 vars
    int count;
} SymbolTable;

// Helper to find variable
int find_var(const SymbolTable* table, const char* name) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->vars[i].name, name) == 0) return i;
    }
    return -1;
}