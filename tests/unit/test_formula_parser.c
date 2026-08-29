#include "formula_parser.h"
#include "formula.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

static FILE *open_text(const char *text) {
    return fmemopen((void *) text, strlen(text), "r");
}

int main(void) {
    const char *input1 =
        "pizza sushi risotto torta\n"
        "-pizza sushi\n"
        "sushi risotto -pizza\n";
    FILE *file1 = open_text(input1);
    Formula formula1;
    assert(formula_parse(file1, &formula1));
    assert(formula1.variable_count == 4);
    assert(formula1.clause_count == 2);
    fclose(file1);
    formula_destroy(&formula1);

    const char *input2 = "sushi\nsushi\nsushi -sushi\n";
    FILE *file2 = open_text(input2);
    Formula formula2;
    assert(formula_parse(file2, &formula2));
    assert(formula2.variable_count == 1);
    assert(formula2.clause_count == 2);
    assert(formula2.clauses[1].is_tautology);
    fclose(file2);
    formula_destroy(&formula2);

    const char *input3 = "pizza sushi\nrisotto\n";
    FILE *file3 = open_text(input3);
    Formula formula3;
    assert(!formula_parse(file3, &formula3));
    fclose(file3);

    const char *input4 = "pizza pizza pizza\npizza\n";
    FILE *file4 = open_text(input4);
    Formula formula4;
    assert(formula_parse(file4, &formula4));
    assert(formula4.variable_count == 1);
    assert(formula4.clause_count == 1);
    assert(formula4.clauses[0].size == 1);
    fclose(file4);
    formula_destroy(&formula4);

    printf("test_formula_parser: OK\n");
    return 0;
}
