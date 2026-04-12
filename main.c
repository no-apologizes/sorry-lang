// Refactoring the entirety of main.c AND adding an AST
// sorry-lang/Junk/ughhhh.png

#include "Headers/lexer.h"
#include "Headers/parser.h"
#include "Headers/symbol_table.h"
#include "Headers/codegen.h"

int main() {
    const char *input = "x 5 = y 10 = x y +";

    // Build AST
    ASTNode* root = build_ast(input);

    SymbolTable table = {.count = 0}; // Default to 0 to be safe

    // Standard LLVM setup
    LLVMContextRef ctx = LLVMContextCreate();
    LLVMModuleRef mod = LLVMModuleCreateWithNameInContext("sorry_lang", ctx);
    LLVMBuilderRef builder = LLVMCreateBuilderInContext(ctx);

    // Create main function wrapper in IR
    LLVMTypeRef main_type = LLVMFunctionType(LLVMInt64TypeInContext(ctx), NULL, 0, 0);
    LLVMValueRef main_func = LLVMAddFunction(mod, "main", main_type);
    LLVMPositionBuilderAtEnd(builder, LLVMAppendBasicBlockInContext(ctx, main_func, "entry"));

    // Generate code
    // Recursively walks tree and fills symbol table
    LLVMValueRef last_val = codegen_visitor(root, builder, ctx, &table);

    // Return last evaluated value
    LLVMBuildRet(builder, last_val);

    // Dump IR
    char *ir = LLVMPrintModuleToString(mod);
    printf("%s", ir);

    LLVMDisposeMessage(ir);
    LLVMDisposeBuilder(builder);
    LLVMDisposeModule(mod);
    LLVMContextDispose(ctx);
    free_node(root); // Use the function in ast.h to prevent memory leaks

    return 0;
}