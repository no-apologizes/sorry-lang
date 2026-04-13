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
    while ((t = get_next_token(&input)).type != TOKEN_EOF) {
        if (top >= 1022) { fprintf(stderr, "Stack overflow\n"); exit(EXIT_FAILURE); }
        switch (t.type) {
            case TOKEN_NUMBER: {
                ASTNode* n = create_node(NODE_NUMBER);
                (*n).value = t.value;
                stack[++top] = n;
                break;
            }
            case TOKEN_IDENTIFIER: {
                ASTNode* n = create_node(NODE_IDENTIFIER);
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
                if (top >= 1) {
                    (*n).right = stack[top--]; // Pop
                    (*n).left = stack[top--]; // Pop
                }
                stack[++top] = n; // Push
                break;
            }
            case TOKEN_EQUALS: {
                ASTNode* n = create_node(NODE_ASSIGN);
                ASTNode* val = stack[top--];
                ASTNode* var = stack[top--];

                // In RPN order will make or break
                // Value before target as in x 5 =,
                // = is the target and x and 5 will to pushed to stack
                // And they are evaluated with =
                (*n).right = val; // Value
                (*n).left = var; // Target
                stack[++top] = n; // Push
                break;
            }
            case TOKEN_TERM: {
                // | folds current stack into one node and keeps going
                ASTNode* folded = stack[0];
                for (int i = 1; i <= top; i++) {
                    ASTNode* seq = create_node(NODE_SEQUENCE);
                    seq->left = folded;
                    seq->right = stack[i];
                    folded = seq;
                }
                top = 0;
                stack[0] = folded;
                break;
            }
            default: break;
        }
    }
    // Fold remaining stack items into a sequence tree
    ASTNode* result = stack[0];
    for (int i = 1; i <= top; i++) {
        ASTNode* seq = create_node(NODE_SEQUENCE);
        seq->left = result;
        seq->right = stack[i];
        result = seq;
    }
    return result;
}