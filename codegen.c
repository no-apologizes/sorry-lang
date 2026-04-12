#include "Headers/codegen.h"

#include <llvm-c/Core.h>

#include "Headers/lexer.h"
#include "Headers/symbol_table.h"


// Omg it's too much work to replace every -> with (*).
LLVMValueRef codegen_visitor(ASTNode* node, LLVMBuilderRef bldr, LLVMContextRef ctx, SymbolTable* table ) {
    if (!node) return NULL; // if not a node
    switch (node->type) {
        case NODE_NUMBER: return LLVMConstInt(LLVMInt64TypeInContext(ctx), node->value, 1);
        case NODE_IDENTIFIER: return LLVMBuildLoad2(bldr, LLVMInt64TypeInContext(ctx), table->vars[find_var(table, node->name)].ptr, "load_tmp");
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
            LLVMBuildStore(bldr, val, table->vars[find_var(table, node->left->name)].ptr);
            return val;
        }
        default: return NULL;
    }
}