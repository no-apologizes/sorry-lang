#pragma once
#include <llvm-c/Core.h>

// Toolkit for generating LLVM IR instructions
// Helper functions but that's what a toolkit is right?
// Used in the parser to build IR from tokens
// TODO: Add more things than just signed 64-bit integers
// Haha big words
// lhs and rhs stand for left and right-hand sides of the operation

// I'm like 60% sure 'ctx' stands for context
// no 'inline' because it breaks with LLVM for some reason
// I disabled the thing where it makes it yellow and yells at you
LLVMValueRef gen_number(LLVMContextRef ctx, int n) { // Generate an i64 constant
    return LLVMConstInt(LLVMInt64TypeInContext(ctx), n, 0); // Why is "SignExtend" 0?
    // Changed i32 to i64
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

// TODO: Add support for other types and operations