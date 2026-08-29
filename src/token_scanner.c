#include "token_scanner.h"
#include <stdlib.h>

#define TOKEN_LIST_INITIAL_CAPACITY 8

void tokenize_line(char *line, TokenList *out) {
    size_t capacity = TOKEN_LIST_INITIAL_CAPACITY;
    out->tokens = malloc(capacity * sizeof(char *));
    out->count = 0;

    char *cursor = line;
    while (*cursor != '\0') {
        while (*cursor == ' ' || *cursor == '\t') {
            cursor++;
        }
        if (*cursor == '\0') {
            break;
        }
        if (out->count == capacity) {
            capacity *= 2;
            out->tokens = realloc(out->tokens, capacity * sizeof(char *));
        }
        out->tokens[out->count] = cursor;
        out->count++;
        while (*cursor != '\0' && *cursor != ' ' && *cursor != '\t') {
            cursor++;
        }
        if (*cursor != '\0') {
            *cursor = '\0';
            cursor++;
        }
    }
}

void token_list_destroy(TokenList *list) {
    free(list->tokens);
    list->tokens = NULL;
    list->count = 0;
}
