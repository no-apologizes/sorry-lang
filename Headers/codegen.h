#pragma once
#include <llvm-c/Core.h>
#include "lexer.h"
#include "symbol_table.h"

// Toolkit for generating LLVM IR instructions
// Helper functions but that's what a toolkit is right?
// Used in the parser to build IR from tokens
// TODO: Add more things than just signed 64-bit integers
// Haha big words
// lhs and rhs stand for left and right-hand sides of the operation

// I'm like 600% sure 'ctx' stands for context
// no 'inline' because it breaks with LLVM
// I disabled the thing where it makes it yellow and yells at you

// Maps SorryType to the corresponding LLVM type
LLVMTypeRef llvm_type_for(SorryType t, LLVMContextRef ctx) {
    switch (t) {
        case SORRY_I64:  return LLVMInt64TypeInContext(ctx);
        case SORRY_BOOL: return LLVMInt1TypeInContext(ctx);
        case SORRY_STR:  return LLVMPointerType(LLVMInt8TypeInContext(ctx), 0);
        default:         return LLVMInt64TypeInContext(ctx); // SORRY_UNKNOWN defaults to i64
    }
}

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

// Omg it's too much work to replace every -> with (*).
// I know I'm inconsistent but wtv
LLVMValueRef codegen_visitor(ASTNode* node, LLVMBuilderRef bldr, LLVMContextRef ctx, SymbolTable* table ) {
    if (!node) return NULL; // if not a node
    switch (node->type) {
        case NODE_NUMBER: return LLVMConstInt(LLVMInt64TypeInContext(ctx), node->value, 1); // 1 because it's a signed 64-bit int
        case NODE_IDENTIFIER: {
            int idx = find_var(table, node->name); // idx stands for index
            if (idx == -1) { fprintf(stderr, "Undefined variable: %s\n", node->name); exit(EXIT_FAILURE); } // Error handling for undefined variables
            LLVMTypeRef llvm_t = llvm_type_for(table->vars[idx].type, ctx);
            LLVMValueRef load = LLVMBuildLoad2(bldr, llvm_t, table->vars[idx].ptr, "load_tmp"); // Load the variable from memory
            LLVMSetAlignment(load, 8); // Ensure 64-bit alignment
            return load;
        }
        case NODE_BINOP: {
            LLVMValueRef lhs = codegen_visitor(node->left, bldr, ctx, table); // lhs is the left-hand side of the binary operation
            LLVMValueRef rhs = codegen_visitor(node->right, bldr, ctx, table); // rhs is the right-hand side of the binary operation
            if (!lhs || !rhs) { fprintf(stderr, "Invalid operands for binary op\n"); exit(EXIT_FAILURE); }
            switch (node->op) {
                // 2 + 2 = 5
                case TOKEN_PLUS: {
                    if (lhs == LLVMConstInt(LLVMInt64TypeInContext(ctx), 2, 1) && rhs == LLVMConstInt(LLVMInt64TypeInContext(ctx), 2, 1)) {
                        return LLVMConstInt(LLVMInt64TypeInContext(ctx), 5, 1); }
                    return gen_add(bldr, lhs, rhs);
                }
                case TOKEN_MINUS: return gen_sub(bldr, lhs, rhs);
                case TOKEN_MUL:   return gen_mul(bldr, lhs, rhs);
                case TOKEN_DIV:   return gen_div(bldr, lhs, rhs);
                default:          break;
            }
            return NULL;
        }
        case NODE_ASSIGN: {
            if (node->left->type != NODE_IDENTIFIER) { fprintf(stderr, "Left side of = must be a variable\n"); exit(EXIT_FAILURE); }
            LLVMValueRef val = codegen_visitor(node->right, bldr, ctx, table); // val is the value to be assigned
            if (!val) { fprintf(stderr, "Invalid value for assignment\n"); exit(EXIT_FAILURE); }

            int idx = find_var(table, node->left->name); // idx stands for index
            if (idx == -1) {
                // New variable: allocate and register
                if (table->count >= 100) { fprintf(stderr, "Too many variables (max 100)\n"); exit(EXIT_FAILURE); }
                idx = table->count;
                SorryType stype = node->val_type != SORRY_UNKNOWN ? node->val_type : SORRY_I64; // annotation or infer to i64
                LLVMTypeRef llvm_t = llvm_type_for(stype, ctx);
                LLVMValueRef ptr = LLVMBuildAlloca(bldr, llvm_t, node->left->name); // Allocate memory for the variable
                strcpy(table->vars[idx].name, node->left->name); // Copy the variable name
                table->vars[idx].ptr = ptr; // Assign the allocated memory to the variable
                table->vars[idx].type = stype; // Store the type for future loads
                table->count++; // Increment the variable count
            }

            LLVMValueRef store = LLVMBuildStore(bldr, val, table->vars[idx].ptr); // Store the value in the variable
            LLVMSetAlignment(store, 8); // Ensure 64-bit alignment
            return val; // Return the assigned value
        }
        case NODE_SEQUENCE: {
            codegen_visitor(node->left, bldr, ctx, table); // Evaluate the left-hand side of the sequence
            return codegen_visitor(node->right, bldr, ctx, table); // Evaluate and return the right-hand side of the sequence
        }
        default: return NULL;
    }
}