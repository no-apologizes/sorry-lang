#include <stdio.h>
#include <stdlib.h>
#include <llvm-c/Core.h>
#include "Headers/lexer.h"
#include "Headers/codegen.h"

void parse_and_gen(const char *input) {
    LLVMContextRef ctx = LLVMContextCreate(); // Create LLVM context
    LLVMModuleRef mod = LLVMModuleCreateWithNameInContext("sorry_lang", ctx); // Module that holds the IR code
    LLVMBuilderRef builder = LLVMCreateBuilderInContext(ctx); // Builder for inserting instructions

    // Define the main function signature: i32 main() with no args
    LLVMTypeRef main_type = LLVMFunctionType(LLVMInt32TypeInContext(ctx), NULL, 0, 0);
    LLVMValueRef main_func = LLVMAddFunction(mod, "main", main_type);

    // Position the builder at the entry block of the main function
    LLVMPositionBuilderAtEnd(builder, LLVMAppendBasicBlockInContext(ctx, main_func, "entry"));

    LLVMValueRef stack[10]; // Fixed-size array TODO: Replace with dynamic stack management
    int top = -1; // Stack pointer starts empty

    Token t;

    while ((t = get_next_token(&input)).type != TOKEN_EOF) {
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
                default: break;
            }
            // Add cases for other vars
        }

    // Return the final result from the stack
    LLVMBuildRet(builder, stack[top]); // 'Ret' stands for 'Return' and returns the value from the function

    // Print generated IR
    char *ir = LLVMPrintModuleToString(mod);
    printf("%s", ir); // '\n' if needed for formatting

    // Clean up LLVM resources
    LLVMDisposeMessage(ir); // Dispose message bc it's a string
    LLVMDisposeBuilder(builder);
    LLVMDisposeModule(mod);
    LLVMContextDispose(ctx);
}

int main() {
    const char *placeholder = "1000 213 *";
    parse_and_gen(placeholder);
    return 0;
}