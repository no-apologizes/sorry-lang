#pragma once
#include <llvm-c/Core.h>
#include "lexer.h"
#include "symbol_table.h"

// Toolkit for generating LLVM IR instructions
// Helper functions but that's what a toolkit is right?
// Used in the parser to build IR from tokens
// Haha big words
// lhs and rhs stand for left and right-hand sides of the operation


// I'm like 600% sure 'ctx' stands for context
// no 'inline' because it breaks with LLVM

// Maps SorryType to the corresponding LLVM type
LLVMTypeRef llvm_type_for(SorryType t, LLVMContextRef ctx) {
    switch (t) {
        case SORRY_I64:  return LLVMInt64TypeInContext(ctx);
        case SORRY_F32:  return LLVMFloatTypeInContext(ctx);
        case SORRY_F64:  return LLVMDoubleTypeInContext(ctx);
        case SORRY_BOOL: return LLVMInt1TypeInContext(ctx);
        case SORRY_STR:  return LLVMPointerType(LLVMInt8TypeInContext(ctx), 0);
        default:         return LLVMInt64TypeInContext(ctx); // SORRY_UNKNOWN defaults to i64
    }
}

LLVMValueRef gen_add(LLVMBuilderRef b, LLVMValueRef lhs, LLVMValueRef rhs)  { return LLVMBuildAdd(b,  lhs, rhs, "add");  }
LLVMValueRef gen_fadd(LLVMBuilderRef b, LLVMValueRef lhs, LLVMValueRef rhs) { return LLVMBuildFAdd(b, lhs, rhs, "fadd"); }
LLVMValueRef gen_sub(LLVMBuilderRef b, LLVMValueRef lhs, LLVMValueRef rhs)  { return LLVMBuildSub(b,  lhs, rhs, "sub");  }
LLVMValueRef gen_fsub(LLVMBuilderRef b, LLVMValueRef lhs, LLVMValueRef rhs) { return LLVMBuildFSub(b, lhs, rhs, "fsub"); }
LLVMValueRef gen_mul(LLVMBuilderRef b, LLVMValueRef lhs, LLVMValueRef rhs)  { return LLVMBuildMul(b,  lhs, rhs, "mul");  }
LLVMValueRef gen_fmul(LLVMBuilderRef b, LLVMValueRef lhs, LLVMValueRef rhs) { return LLVMBuildFMul(b, lhs, rhs, "fmul"); }
LLVMValueRef gen_div(LLVMBuilderRef b, LLVMValueRef lhs, LLVMValueRef rhs)  { return LLVMBuildSDiv(b, lhs, rhs, "div");  }
LLVMValueRef gen_fdiv(LLVMBuilderRef b, LLVMValueRef lhs, LLVMValueRef rhs) { return LLVMBuildFDiv(b, lhs, rhs, "fdiv"); }

LLVMValueRef codegen_visitor(ASTNode* node, LLVMBuilderRef bldr, LLVMContextRef ctx, SymbolTable* table) {
    if (!node) return NULL;
    switch (node->type) {

        case NODE_NUMBER:
            // val_type is set when a type keyword annotated the literal (e.g. `i64 2`, `f32 3.5`)
            // SORRY_UNKNOWN means fall back to what the literal looks like
            switch (node->val_type) {
                case SORRY_F32:  return LLVMConstReal(LLVMFloatTypeInContext(ctx),  node->float_value);
                case SORRY_F64:  return LLVMConstReal(LLVMDoubleTypeInContext(ctx), node->float_value);
                case SORRY_I64:  return LLVMConstInt(LLVMInt64TypeInContext(ctx),   node->value, 1);
                case SORRY_BOOL: return LLVMConstInt(LLVMInt1TypeInContext(ctx),    node->value != 0, 0);
                default: // infer from literal: is_float → double, else i64
                    if (node->is_float)
                        return LLVMConstReal(LLVMDoubleTypeInContext(ctx), node->float_value);
                    return LLVMConstInt(LLVMInt64TypeInContext(ctx), node->value, 1);
            }

        case NODE_IDENTIFIER: {
            int idx = find_var(table, node->name);
            if (idx == -1) { fprintf(stderr, "Undefined variable: %s\n", node->name); exit(EXIT_FAILURE); }
            LLVMTypeRef llvm_t = llvm_type_for(table->vars[idx].type, ctx);
            LLVMValueRef load = LLVMBuildLoad2(bldr, llvm_t, table->vars[idx].ptr, "load_tmp");
            LLVMSetAlignment(load, 8);
            return load;
        }

        case NODE_BINOP: {
            LLVMValueRef lhs = codegen_visitor(node->left,  bldr, ctx, table);
            LLVMValueRef rhs = codegen_visitor(node->right, bldr, ctx, table);
            if (!lhs || !rhs) { fprintf(stderr, "Invalid operands for binary op\n"); exit(EXIT_FAILURE); }
            LLVMTypeRef lhs_t = LLVMTypeOf(lhs);
            LLVMTypeRef rhs_t = LLVMTypeOf(rhs);
            if (lhs_t != rhs_t) {
                fprintf(stderr, "Type mismatch in binary op: %s vs %s\n",
                    LLVMPrintTypeToString(lhs_t), LLVMPrintTypeToString(rhs_t));
                exit(EXIT_FAILURE);
            }
            LLVMTypeKind kind = LLVMGetTypeKind(lhs_t);
            int is_fp = kind == LLVMFloatTypeKind || kind == LLVMDoubleTypeKind;
            switch (node->op) {
                case TOKEN_PLUS:  return is_fp ? gen_fadd(bldr, lhs, rhs) : gen_add(bldr, lhs, rhs);
                case TOKEN_MINUS: return is_fp ? gen_fsub(bldr, lhs, rhs) : gen_sub(bldr, lhs, rhs);
                case TOKEN_MUL:   return is_fp ? gen_fmul(bldr, lhs, rhs) : gen_mul(bldr, lhs, rhs);
                case TOKEN_DIV:   return is_fp ? gen_fdiv(bldr, lhs, rhs) : gen_div(bldr, lhs, rhs);
                default:          break;
            }
            return NULL;
        }

        case NODE_ASSIGN: {
            if (node->left->type != NODE_IDENTIFIER) { fprintf(stderr, "Left side of = must be a variable\n"); exit(EXIT_FAILURE); }
            LLVMValueRef val = codegen_visitor(node->right, bldr, ctx, table);
            if (!val) { fprintf(stderr, "Invalid value for assignment\n"); exit(EXIT_FAILURE); }

            int idx = find_var(table, node->left->name);
            SorryType declared = node->left->val_type; // SORRY_UNKNOWN for bare `x =`
            LLVMTypeRef val_t  = LLVMTypeOf(val);
            LLVMTypeRef llvm_t;
            SorryType stype;

            if (idx == -1) {
                // New variable declaration
                if (declared != SORRY_UNKNOWN) {
                    // `i64 x = ...` — type is explicit, value must match exactly
                    stype  = declared;
                    llvm_t = llvm_type_for(stype, ctx);
                    if (val_t != llvm_t) {
                        fprintf(stderr, "Type mismatch: cannot assign %s value to declared %s variable '%s'\n",
                            LLVMPrintTypeToString(val_t), LLVMPrintTypeToString(llvm_t), node->left->name);
                        exit(EXIT_FAILURE);
                    }
                } else {
                    // `x = ...` — infer type from what the value actually is
                    llvm_t = val_t;
                    LLVMTypeKind k = LLVMGetTypeKind(llvm_t);
                    if      (k == LLVMFloatTypeKind)   stype = SORRY_F32;
                    else if (k == LLVMDoubleTypeKind)  stype = SORRY_F64;
                    else                               stype = SORRY_I64;
                }
                if (table->count >= 100) { fprintf(stderr, "Too many variables (max 100)\n"); exit(EXIT_FAILURE); }
                idx = table->count;
                LLVMValueRef ptr = LLVMBuildAlloca(bldr, llvm_t, node->left->name);
                strcpy(table->vars[idx].name, node->left->name);
                table->vars[idx].ptr  = ptr;
                table->vars[idx].type = stype;
                table->count++;
            } else {
                // Re-assignment — type must match what the variable was declared as
                stype  = table->vars[idx].type;
                llvm_t = llvm_type_for(stype, ctx);
                if (val_t != llvm_t) {
                    fprintf(stderr, "Type mismatch: cannot assign %s value to %s variable '%s'\n",
                        LLVMPrintTypeToString(val_t), LLVMPrintTypeToString(llvm_t), node->left->name);
                    exit(EXIT_FAILURE);
                }
            }
            LLVMValueRef store = LLVMBuildStore(bldr, val, table->vars[idx].ptr);
            LLVMSetAlignment(store, 8);
            return val;
        }

        case NODE_SEQUENCE: {
            codegen_visitor(node->left,  bldr, ctx, table);
            return codegen_visitor(node->right, bldr, ctx, table);
        }

        default: return NULL;
    }
}
