#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"

// Arrows are stupid to type
// Yes I am inconsistent but wtv
ASTNode* create_node(NodeType type) {
    ASTNode* node = calloc(1, sizeof(ASTNode)); // calloc zeroes memory, no manual defaults needed
    if (!node) { // If not a node
        fprintf(stderr, "Memory allocation failed\n"); // Wonderful error handling
        exit(EXIT_FAILURE); // exit(1);
    }
    (*node).type = type;
    return node;
}

// Clean up to prevent mem leaks
void free_node(ASTNode* node) {
    if (!node) return; // Not needed if it's not a node
    free_node(node->left);
    free_node(node->right);
    free(node);
}