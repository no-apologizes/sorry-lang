#include <stdio.h>
#include <string.h>
#include <llvm-c/Core.h>
#include "Headers/lexer.h"
#include "Headers/codegen.h"

// DD/MM/YYYY
// 11/04/2026
// I'm making a symbol table now

void parse_and_gen(const char *input) {
    LLVMContextRef ctx = LLVMContextCreate(); // Create LLVM context
    LLVMModuleRef mod = LLVMModuleCreateWithNameInContext("sorry_lang", ctx); // Module that holds the IR code
    LLVMBuilderRef builder = LLVMCreateBuilderInContext(ctx); // Builder for inserting instructions

    // Define the main function signature: i64 main() with no args
    LLVMTypeRef main_type = LLVMFunctionType(LLVMInt64TypeInContext(ctx), NULL, 0, 0);
    LLVMValueRef main_func = LLVMAddFunction(mod, "main", main_type);

    // Position the builder at the entry block of the main function
    LLVMPositionBuilderAtEnd(builder, LLVMAppendBasicBlockInContext(ctx, main_func, "entry"));

    LLVMValueRef stack[10]; // Fixed-size array TODO: Replace with dynamic stack management
    int top = -1; // Stack pointer starts empty

    Token t;

    // Parallel array work fine for saving small variable counts
    //LLVMValueRef vars[26]; // 26 because some people only allow a-z to start
    // rip vars, you will be missed
    char var_names[26][32];
    char pending_name[32] = {0};
    int var_count = 0;
    // Wow, that's a lot of non-dynamic arrays
    LLVMValueRef var_ptrs[26]; // Store memory addrs (pointers)



    while ((t = get_next_token(&input)).type != TOKEN_EOF && t.type != TOKEN_TERM) { // | is the terminator
        switch (t.type) {
                case (TOKEN_NUMBER): stack[++top] = gen_number(ctx, (t.value)); // Push a number as an LLVM constant onto the stack
                break;
                case TOKEN_PLUS: {
                    LLVMValueRef a = stack[top--];
                    LLVMValueRef b = stack[top--];
                    stack[++top] = LLVMBuildAdd(builder, a, b, "add");
                }
                break;
                case TOKEN_MINUS: {
                    // Pop b first to keep the math mathing
                    LLVMValueRef b = stack[top--];
                    LLVMValueRef a = stack[top--];
                    stack[++top] = LLVMBuildSub(builder, a, b, "sub");
                }
                break;
                case TOKEN_MUL: {
                    LLVMValueRef a = stack[top--];
                    LLVMValueRef b = stack[top--];
                    stack[++top] = LLVMBuildMul(builder, a, b, "mul");
                }
                break;
                case TOKEN_DIV: {
                    // Pop b first to keep the math mathing
                    LLVMValueRef b = stack[top--]; // rhs (divisor)
                    LLVMValueRef a = stack[top--]; // lhs (dividend)
                    // Check for division by at runtime (Do it before at like compile time or whatever it's called)
                    // if (b == 0) {
                    //     fprintf(stderr, "Division by zero\n");
                    //     exit(1); // Add a cleaner way to handle errors
                    // }
                    stack[++top] = gen_div(builder, a, b);
                }
                break; // Forgot to add a stupid ass break here
                case TOKEN_IDENTIFIER : {
                    // Check if name already exists
                    int found = -1;
                    for (int i = 0; i < var_count; i++) {
                        if (strcmp(var_names[i], t.name) == 0) { found = i; break;}
                    }

                    if (found >= 0) {
                        // Generate a 'load' from the pointer onto the stack
                        stack[++top] = LLVMBuildLoad2(builder, LLVMInt64TypeInContext(
                            ctx), var_ptrs[found], "load_tmp");
                    }
                    else {
                        // Might be an lvalue so save name for TOKEN_EQUALS
                        strcpy(pending_name, t.name);
                    }
                }
                break;
                case TOKEN_EQUALS: {
                    // RPN assignment so <var> <name> =

                    LLVMValueRef value_to_store = stack[top--]; // Pop

                    // Check is this variable already has a memory slot (alloca)
                    int found = -1;
                    for (int i = 0; i < var_count; i++) {
                        if (strcmp(var_names[i], pending_name) == 0) { found = i; break; }
                    }

                    if (found == -1) {
                        // Allocate memory for the variable
                        var_ptrs[var_count] = LLVMBuildAlloca(builder, LLVMInt64TypeInContext(ctx), pending_name);
                        found = var_count++;
                        strcpy(var_names[found], pending_name);
                    }

                    // Store the value to the ptr
                    LLVMBuildStore(builder, value_to_store, var_ptrs[found]);

                    // Push the stored value back onto the stack
                    stack[++top] = value_to_store;
                }
                break;
                default: break;
            }
            // Add cases for other vars
        }

    // Return the final result from the stack
    LLVMBuildRet(builder, stack[top]); // 'Ret' stands for 'Return' and returns the value from the function

    // Print-generated IR
    char *ir = LLVMPrintModuleToString(mod);
    printf("%s", ir); // '\n' if needed for formatting

    // Clean up LLVM resources
    LLVMDisposeMessage(ir); // Dispose message bc it's a string
    LLVMDisposeBuilder(builder);
    LLVMDisposeModule(mod);
    LLVMContextDispose(ctx);
}

int main() {
    const char *placeholder = "x 5 = y 4 = y x +";
    parse_and_gen(placeholder);
    return 0;
}