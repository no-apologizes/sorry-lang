#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen.h"
#include "lexer.h"
#include "ast.h"
#include "symbol_table.h"

ASTNode* build_ast(const char *input) {
    ASTNode* stack[1024]; // uh idk
    int top = -1; // Always -1, bro
    Token t;

    // #define TKN(t) ((Token){.type = (t)}) doesn't work, and I don't care to figure why
    while ((t = get_next_token(&input)).type != TOKEN_EOF && t.type != TOKEN_TERM) {
        switch (t.type) {
            case TOKEN_NUMBER: {
                ASTNode* n = create_node(TOKEN_NUMBER);
                (*n).value = t.value;
                stack[++top] = n;
                break;
            }
            case TOKEN_IDENTIFIER: {
                ASTNode* n = create_node(TOKEN_IDENTIFIER);
                strcpy((*n).name, t.name);
                stack[++top] = n;
                break;
            }
            case TOKEN_PLUS:
            case TOKEN_MINUS:
            case TOKEN_MUL:
            case TOKEN_DIV: {
                ASTNode* n = create_node(NODE_BINOP);
                (*n).op = t.type; // Store token type as operator
                (*n).left = stack[top--]; // Pop
                (*n).right = stack[top--]; // Pop
                stack[++top] = n; // Push
                break;
            }
            case TOKEN_EQUALS: {
                ASTNode* n = create_node(NODE_ASSIGN);
                ASTNode* var = stack[top--];
                ASTNode* val = stack[top--];

                // In RPN order will make or break
                // Value before target as in x 5 =,
                // = is the target and x and 5 will to pushed to stack
                // And they are evaluated with =
                (*n).right = val; // Value
                (*n).left = var; // Target
                stack[++top] = n; // Push
                break;
            }
            default: break;
        }
    }
    return stack[top]; // Return the final AST node
}