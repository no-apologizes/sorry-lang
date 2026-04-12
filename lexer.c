#include "Headers/lexer.h"
#include <ctype.h>
#define TKN(t) ((Token){.type = (t)})// So don't have to put (Token) in-front of everything
// That was weird to figure out

Token get_next_token(const char **input) {
    while (isspace(**input)) (*input)++; // Skip whitespaces

    if (**input == '\0') return TKN(TOKEN_EOF); // THIS LINE 'TKN(TOKEN_EOF)'
    // That's lowkey stupid, so I'll probably change that

    if (isdigit(**input)) {
        Token t = {TOKEN_NUMBER, 0}; // Default to 0
        while (isdigit(**input)) {
            t.value = t.value * 10 + (**input - '0');
            (*input)++; // Consume the digit
        }
        return t;
    }

    if (isalpha(**input)) {
        Token t = {TOKEN_IDENTIFIER};
        int i = 0; // Default to 0
        while (isalnum(**input)) t.name[i++] = *(*input)++;
        t.name[i] = '\0';
        return t;
    }

    char c = *(*input)++;
    switch (c) {
            case '+': return TKN(TOKEN_PLUS);
            case '-': return TKN(TOKEN_MINUS);
            case '*': return TKN(TOKEN_MUL);
            case '/': return TKN(TOKEN_DIV);
            case '=': return TKN(TOKEN_EQUALS);
            case '|': return TKN(TOKEN_TERM);
            default: return TKN(TOKEN_EOF);
    }
}