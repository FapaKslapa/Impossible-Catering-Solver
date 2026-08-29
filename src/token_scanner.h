#ifndef TOKEN_SCANNER_H
#define TOKEN_SCANNER_H

#include <stddef.h>

typedef struct {
    char **tokens;
    size_t count;
} TokenList;

void tokenize_line(char *line, TokenList *out);
void token_list_destroy(TokenList *list);

#endif
