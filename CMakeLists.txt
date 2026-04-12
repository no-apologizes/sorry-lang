cmake_minimum_required(VERSION 4.1)
project(sorry C)

set(CMAKE_C_STANDARD 17)
#set(CMAKE_C_STANDARD_REQUIRED ON)

find_program(LLVM_CONFIG_EXECUTABLE llvm-config REQUIRED)

execute_process(
        COMMAND ${LLVM_CONFIG_EXECUTABLE} --cflags
        OUTPUT_VARIABLE LLVM_CFLAGS
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
        COMMAND ${LLVM_CONFIG_EXECUTABLE} --ldflags
        OUTPUT_VARIABLE LLVM_LDFLAGS
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
        COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs core analysis
        OUTPUT_VARIABLE LLVM_LIBS
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

separate_arguments(LLVM_CFLAGS_LIST NATIVE_COMMAND "${LLVM_CFLAGS}")
separate_arguments(LLVM_LDFLAGS_LIST NATIVE_COMMAND "${LLVM_LDFLAGS}")
separate_arguments(LLVM_LIBS_LIST NATIVE_COMMAND "${LLVM_LIBS}")

add_executable(sorry main.c
        Headers/lexer.h
        Headers/codegen.h
        lexer.c
        ast.c)

target_compile_options(sorry PRIVATE ${LLVM_CFLAGS_LIST})
target_link_options(sorry PRIVATE ${LLVM_LDFLAGS_LIST})
target_link_libraries(sorry PRIVATE ${LLVM_LIBS_LIST})