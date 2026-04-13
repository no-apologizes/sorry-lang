// Refactoring the entirety of main.c AND adding an AST
// sorry-lang/Junk/ughhhh.png

#include <stdio.h>
#include <stdlib.h>
#include "Headers/lexer.h"
#include "Headers/parser.h"
#include "Headers/symbol_table.h"
#include "Headers/codegen.h"

int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "Usage: sorry <file.sry>\n"); return 1; }

    FILE *f = fopen(argv[1], "r");
    if (!f) { fprintf(stderr, "Cannot open file: %s\n", argv[1]); return 1; }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    char *src = malloc(size + 1);
    fread(src, 1, size, f);
    src[size] = '\0';
    fclose(f);

    // Build AST
    ASTNode* root = build_ast(src);
    free(src);

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
    printf("%s\n", ir); // Wow, I actually need the newline

    LLVMDisposeMessage(ir);
    LLVMDisposeBuilder(builder);
    LLVMDisposeModule(mod);
    LLVMContextDispose(ctx);
    free_node(root); // Use the function in ast.h to prevent memory leaks

    return 0;
}