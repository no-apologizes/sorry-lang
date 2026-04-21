#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include "Headers/lexer.h"
#include "Headers/parser.h"
#include "Headers/symbol_table.h"
#include "Headers/codegen.h"

int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "Usage: sorry <file.sry>\n"); return 1; } // Check if the filename was provided
    const size_t len = strlen(argv[1]); // Size of the filename
    if (len < 4 || strcmp(argv[1] + len - 4, ".sry") != 0) { fprintf(stderr, "Error: file must end in .sry\n"); return 1; } // Jump to last 4 chars and compare them to '.sry'

    FILE *f = fopen(argv[1], "r"); // Attempt to open file in read-only mode
    if (!f) { fprintf(stderr, "Cannot open file: %s\n", argv[1]); return 1; }
    fseek(f, 0, SEEK_END); // Seek to end of file
    long size = ftell(f); // Get file size
    rewind(f); // Go back to the start
    char *src = malloc(size + 1); // Length of file plus a null terminator for a valid c string
    if (!src) { fprintf(stderr, "Memory allocation failed\n"); fclose(f); return 1; } // If memory allocation fails, clean up and exit
    fread(src, 1, size, f); // Read file contents into src
    src[size] = '\0'; // Null terminate the string
    fclose(f); // Clean up file handle

    // Build AST
    ASTNode* root = build_ast(src);
    free(src); // Free memory allocated for source code

    SymbolTable table = {.count = 0}; // Default to 0 to be safe

    // Standard LLVM setup
    LLVMContextRef ctx = LLVMContextCreate();
    LLVMModuleRef mod = LLVMModuleCreateWithNameInContext("sorry_lang", ctx);
    LLVMBuilderRef builder = LLVMCreateBuilderInContext(ctx);

    // Create main function wrapper in IR
    // LLVMTypeRef main_type = LLVMFunctionType(LLVMDoubleTypeInContext(ctx), NULL, 0, 0);
    LLVMTypeRef main_type = LLVMFunctionType(LLVMInt64TypeInContext(ctx), NULL, 0, 0);
    LLVMValueRef main_func = LLVMAddFunction(mod, "main", main_type);
    LLVMPositionBuilderAtEnd(builder, LLVMAppendBasicBlockInContext(ctx, main_func, "entry"));

    // malloc mem for visitor, as it defaults to null and will just return a segmentation violation
    table.vars = malloc(100 * sizeof(Symbol));

    // Generate code
    // Recursively walks tree and fills symbol table
    LLVMValueRef last_val = codegen_visitor(root, builder, ctx, &table);

    LLVMBuildRet(builder, last_val);

    // Print IR and disassemble
    char *ir = LLVMPrintModuleToString(mod);
    printf("IR:\n%s\n", ir);
    LLVMDisposeMessage(ir);

    LLVMDisposeBuilder(builder);

    // JIT execute
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    LLVMLinkInMCJIT();

    // Build executable
    LLVMExecutionEngineRef engine;
    char *err = NULL;
    if (LLVMCreateExecutionEngineForModule(&engine, mod, &err)) {
        fprintf(stderr, "JIT error: %s\n", err);
        LLVMDisposeMessage(err);
        return 1;
    }

    // Run and print result
    LLVMGenericValueRef result = LLVMRunFunction(engine, main_func, 0, NULL);

    /*
    LLVMTypeKind last_kind = LLVMGetTypeKind(LLVMTypeOf(last_val));
    if (last_kind == LLVMIntegerTypeKind) {
        printf("Actual output:\n\n%lld\n", (long long)LLVMGenericValueToInt(result, 1)); // long long int, negative to positive nine quintillion, 64-bit int limit
    } else {
        double result_d = LLVMGenericValueToFloat(LLVMDoubleTypeInContext(ctx), result);
        printf("Actual output:\n\n%g\n", result_d);
    }
    */

    // Free all resources
    LLVMDisposeGenericValue(result);
    LLVMDisposeExecutionEngine(engine); // also frees mod, engine owns mod now so allow it to dispose
    LLVMContextDispose(ctx);
    free_node(root);

    return 0;
}