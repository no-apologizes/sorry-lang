#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"

// Arrows are stupid to type
// Yes I am inconsistent but wtv
ASTNode* create_node(NodeType type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) { // If not a node
        fprintf(stderr, "Memory allocation failed\n"); // Wonderful error handling
        exit(EXIT_FAILURE); // exit(1);
    }
    // Defaults
    (*node).type = type;
    (*node).left = NULL;
    (*node).right = NULL;
    (*node).value = 0;
    memset((*node).name, 0, sizeof((*node).name));
    return node;
}

// Clean up to prevent mem leaks
void free_node(ASTNode* node) {
    if (!node) return; // Not needed if it's not a node
    free((*node).left);
    free((*node).right);
    free(node);
}