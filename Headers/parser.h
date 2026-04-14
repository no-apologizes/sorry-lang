#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "ast.h"

ASTNode* build_ast(const char *input) {
    int stack_cap = 1024; // Local so each call starts fresh
    ASTNode** stack = malloc(stack_cap * sizeof(ASTNode*));
    if (stack == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    int top = -1; // Always -1, bro
    Token t;

    // #define TKN(t) ((Token){.type = (t)}) doesn't work, and I don't care to figure why
    while ((t = get_next_token(&input)).type != TOKEN_EOF) {
        if (top + 1 >= stack_cap) { // Grow before pushing, not after overflowing
            int new_cap = stack_cap * 2; // Multiply by 2 for exponential growth
            // reallocarray is a BSD/GNU extension, so it won't work on anything else; namely mac and windows
            ASTNode **tmp = reallocarray(stack, new_cap, sizeof(ASTNode*)); // reallocarray handles overflow and memory management, purpose built for this, so you use it instead of realloc
            if (!tmp) { free(stack); fprintf(stderr, "Memory allocation failed\n"); exit(EXIT_FAILURE); }
            stack = tmp;
            stack_cap = new_cap;
        }
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
                if (top < 1) { fprintf(stderr, "Not enough operands for operator\n"); exit(EXIT_FAILURE); }
                ASTNode* n = create_node(NODE_BINOP);
                (*n).op = t.type; // Store token type as operator
                (*n).right = stack[top--]; // Pop
                (*n).left = stack[top--]; // Pop
                stack[++top] = n; // Push
                break;
            }
            case TOKEN_TYPE: {
                ASTNode* n = create_node(NODE_TYPE);
                n->val_type = t.val_type;
                stack[++top] = n;
                break;
            }
            case TOKEN_EQUALS: {
                if (top < 1) { fprintf(stderr, "Not enough operands for =\n"); exit(EXIT_FAILURE); }
                ASTNode* n = create_node(NODE_ASSIGN);
                ASTNode* val = stack[top--];
                // Optional type annotation sits between identifier and value
                if (stack[top]->type == NODE_TYPE) {
                    n->val_type = stack[top]->val_type;
                    free_node(stack[top--]); // consume and discard the type node
                }
                ASTNode* var = stack[top--];
                (*n).right = val; // Value
                (*n).left = var; // Target
                stack[++top] = n; // Push
                break;
            }
            case TOKEN_DROP: {
                if (top < 0) { fprintf(stderr, "DROP on empty stack\n"); exit(EXIT_FAILURE); }
                free_node(stack[top--]);
                break;
            }
            case TOKEN_TERM: {
                if (top < 0) break; // Nothing on stack, ignore
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

    if (top < 0) return NULL; // TODO: Return an error

    ASTNode* result = stack[0];
    for (int i = 1; i <= top; i++) {
        ASTNode* seq = create_node(NODE_SEQUENCE);
        seq->left = result;
        seq->right = stack[i];
        result = seq;
    }
    free(stack); // Stack itself is done, the nodes live on in the tree
    return result;
}