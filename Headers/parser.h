#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "ast.h"

ASTNode* build_ast(const char *input) {
    int stack_cap = 512;
    ASTNode** stack = malloc(stack_cap * sizeof(ASTNode*));
    if (stack == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    int top = -1; // Always -1, bro
    Token t;

    ASTNode* assign_target = NULL;

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
            case TOKEN_TYPE: {
                SorryType declared_type = t.val_type;
                const char* peek = input;
                Token next = get_next_token(&peek);

                if (next.type == TOKEN_IDENTIFIER) {
                    // Handle: i64 x = ...
                    get_next_token(&input); // Consume identifier
                    assign_target = create_node(NODE_IDENTIFIER);
                    strncpy(assign_target->name, next.name, 31);
                    assign_target->val_type = declared_type; // Apply strict type

                    Token eq = get_next_token(&input); // Expect '='
                    if (eq.type != TOKEN_EQUALS) {
                        fprintf(stderr, "Type Error: Expected '=' after typed identifier\n");
                        exit(EXIT_FAILURE);
                    }
                } else if (next.type == TOKEN_NUMBER) {
                    // Handle: x = i64 2
                    get_next_token(&input); // Consume number
                    ASTNode* n = create_node(NODE_NUMBER);
                    n->value = next.value;
                    n->float_value = next.float_value;
                    n->val_type = declared_type; // Force this number to be this type
                    n->is_float = (declared_type == SORRY_F64 || declared_type == SORRY_F32);
                    stack[++top] = n;
                } else {
                    fprintf(stderr, "Type Error: Expected identifier or number after type keyword\n");
                    exit(EXIT_FAILURE);
                }
                break;
            }

            case TOKEN_IDENTIFIER: {
                // Peek at next token to see if it's an =
                const char* peek = input;
                Token next = get_next_token(&peek);

                if (next.type == TOKEN_EQUALS) {
                    // `x = ...` — bare assignment, type will be inferred from the value
                    assign_target = create_node(NODE_IDENTIFIER);
                    strncpy(assign_target->name, t.name, 31);
                    get_next_token(&input); // consume =
                } else {
                    // Just a variable reference, push to stack
                    ASTNode* n = create_node(NODE_IDENTIFIER);
                    strcpy(n->name, t.name);
                    stack[++top] = n;
                }
                break;
            }
            case TOKEN_NUMBER: {
                ASTNode* n = create_node(NODE_NUMBER);
                n->value       = t.value;
                n->float_value = t.float_value;
                n->is_float    = t.is_float;
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
            case TOKEN_DROP: {
                if (top < 0) { fprintf(stderr, "DROP on empty stack\n"); exit(EXIT_FAILURE); }
                free_node(stack[top--]);
                break;
            }
            case TOKEN_TERM: {
                if (top < 0) break; // Nothing on stack, ignore
                // | folds current stack into one node and keeps going
                ASTNode* rpn_result = stack[top--];

                if (assign_target) {
                     ASTNode* assign_node = create_node(NODE_ASSIGN);
                    assign_node->left = assign_target;
                    assign_node->right = rpn_result;
                    stack[++top] = assign_node;
                    assign_target = NULL; // Reset for next line
                } else {
                    stack[++top] = rpn_result;
                }

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