#pragma once
#include <llvm-c/Core.h>
//#include "Headers/codegen.h"
#include "lexer.h"
#include "symbol_table.h"

// Toolkit for generating LLVM IR instructions
// Helper functions but that's what a toolkit is right?
// Used in the parser to build IR from tokens
// TODO: Add more things than just signed 64-bit integers
// Haha big words
// lhs and rhs stand for left and right-hand sides of the operation

// I'm like 60% sure 'ctx' stands for context
// no 'inline' because it breaks with LLVM
// I disabled the thing where it makes it yellow and yells at you

// Rip gen_number

LLVMValueRef gen_add(LLVMBuilderRef b, LLVMValueRef lhs, LLVMValueRef rhs) {
    return LLVMBuildAdd(b, lhs, rhs, "add");
}

LLVMValueRef gen_sub(LLVMBuilderRef b, LLVMValueRef lhs, LLVMValueRef rhs) {
    return LLVMBuildSub(b, lhs, rhs, "sub");
}

LLVMValueRef gen_mul(LLVMBuilderRef b, LLVMValueRef lhs, LLVMValueRef rhs) {
    return LLVMBuildMul(b, lhs, rhs, "mul");
}

LLVMValueRef gen_div(LLVMBuilderRef b, LLVMValueRef lhs, LLVMValueRef rhs) {
    return LLVMBuildSDiv(b, lhs, rhs, "div");
}

// Omg it's too much work to replace every -> with (*)
// I know I'm inconsistent but wtv
LLVMValueRef codegen_visitor(ASTNode* node, LLVMBuilderRef bldr, LLVMContextRef ctx, SymbolTable* table ) {
    if (!node) return NULL; // if not a node
    switch (node->type) {
        case NODE_NUMBER: return LLVMConstInt(LLVMInt64TypeInContext(ctx), node->value, 1);
        case NODE_IDENTIFIER: {
            int idx = find_var(table, node->name); // idx stands for index
            if (idx > -1) {
                LLVMValueRef load = LLVMBuildLoad2(bldr, LLVMInt64TypeInContext(ctx), table->vars[idx].ptr, "load_tmp");
                LLVMSetAlignment(load, 8); // Ensure 64-bit alignment
                return load;
            }
            return NULL;
        }

        case NODE_BINOP: {
            LLVMValueRef lhs = codegen_visitor(node->left, bldr, ctx, table);
            LLVMValueRef rhs = codegen_visitor(node->right, bldr, ctx, table);
            switch (node->op) {
                case TOKEN_PLUS: return gen_add(bldr, lhs, rhs);
                case TOKEN_MINUS: return gen_sub(bldr, lhs, rhs);
                case TOKEN_MUL: return gen_mul(bldr, lhs, rhs);
                case TOKEN_DIV: return gen_div(bldr, lhs, rhs);
                default: break;
            }
            return NULL;
        }
        case NODE_ASSIGN: {
            LLVMValueRef val = codegen_visitor(node->right, bldr, ctx, table);

            int idx = find_var(table, node->left->name);
            if (idx == -1) {
                // New variable: allocate and register
                idx = table->count;
                LLVMValueRef ptr = LLVMBuildAlloca(bldr, LLVMInt64TypeInContext(ctx), node->left->name);
                strcpy(table->vars[idx].name, node->left->name);
                table->vars[idx].ptr = ptr;
                table->count++;
            }

            LLVMValueRef store = LLVMBuildStore(bldr, val, table->vars[idx].ptr);
            LLVMSetAlignment(store, 8);
            return val;
        }
        case NODE_SEQUENCE: {
            codegen_visitor(node->left, bldr, ctx, table);
            return codegen_visitor(node->right, bldr, ctx, table);
        }
        default: return NULL;
    }
}