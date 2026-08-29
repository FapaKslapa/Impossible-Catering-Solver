#include "token_scanner.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    char line1[] = "pizza sushi risotto";
    TokenList tokens1;
    tokenize_line(line1, &tokens1);
    assert(tokens1.count == 3);
    assert(strcmp(tokens1.tokens[0], "pizza") == 0);
    assert(strcmp(tokens1.tokens[1], "sushi") == 0);
    assert(strcmp(tokens1.tokens[2], "risotto") == 0);
    token_list_destroy(&tokens1);

    char line2[] = "  -pizza   sushi  ";
    TokenList tokens2;
    tokenize_line(line2, &tokens2);
    assert(tokens2.count == 2);
    assert(strcmp(tokens2.tokens[0], "-pizza") == 0);
    assert(strcmp(tokens2.tokens[1], "sushi") == 0);
    token_list_destroy(&tokens2);

    char line3[] = "";
    TokenList tokens3;
    tokenize_line(line3, &tokens3);
    assert(tokens3.count == 0);
    token_list_destroy(&tokens3);

    printf("test_token_scanner: OK\n");
    return 0;
}
