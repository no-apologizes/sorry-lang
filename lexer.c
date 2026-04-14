#include "Headers/lexer.h"
#include <ctype.h>
#include <string.h>
#define TKN(t) ((Token){.type = (t)}) // So I don't have to put (Token) in front of everything
// That was weird to figure out

Token get_next_token(const char **input) {
    while (**input == ' ' || **input == '\t' || **input == '\r') (*input)++; // Skip whitespace (not newlines)

    if (**input == '\0') return TKN(TOKEN_EOF); // THIS LINE 'TKN(TOKEN_EOF)'
    // That's lowkey stupid, so I'll probably change that

    if (isdigit(**input)) {
        Token t = {TOKEN_NUMBER, 0L}; // Default to 0
        while (isdigit(**input)) {
            t.value = t.value * 10 + (**input - '0'); // The 10 is here because we are using a base-10-number system
            (*input)++; // Consume the digit
        }
        return t;
    }

    if (isalpha(**input)) {
        Token t = {TOKEN_IDENTIFIER};
        int i = 0; // Default to 0
        while (isalnum(**input) && i < 31) t.name[i++] = *(*input)++; // Cap at 31 to leave room for null terminator
        while (isalnum(**input)) (*input)++; // Consume overflow chars without writing them
        t.name[i] = '\0';
        if      (strcmp(t.name, "i64")  == 0) { t.type = TOKEN_TYPE; t.val_type = SORRY_I64;  }
        else if (strcmp(t.name, "bool") == 0) { t.type = TOKEN_TYPE; t.val_type = SORRY_BOOL; }
        else if (strcmp(t.name, "str")  == 0) { t.type = TOKEN_TYPE; t.val_type = SORRY_STR;  }
        return t;
    }

    char c = *(*input)++;
    switch (c) {
        case '+':  return TKN(TOKEN_PLUS);
        case '-':  return TKN(TOKEN_MINUS);
        case '*':  return TKN(TOKEN_MUL);
        case '/':  return TKN(TOKEN_DIV);
        case '=':  return TKN(TOKEN_EQUALS);
        case ';':  return TKN(TOKEN_DROP);
        case '|':
        case '\n': return TKN(TOKEN_TERM); // Why is the \n here?
        default:   return get_next_token(input); // Skip unknown chars instead of ending parse
    }
}