#include "threshold_search.h"
#include "formula.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    int pizza = 0, sushi = 1, risotto = 2;

    int pool1[5];
    pool1[0] = lit_neg(pizza); pool1[1] = lit_pos(sushi);
    pool1[2] = lit_pos(sushi); pool1[3] = lit_pos(risotto); pool1[4] = lit_neg(pizza);
    Clause clauses1[] = {
        {0, 2, false},
        {2, 3, false}
    };
    Formula formula1;
    formula1.variable_count = 4;
    formula1.literal_pool = pool1;
    formula1.literal_pool_size = 5;
    formula1.clauses = clauses1;
    formula1.clause_count = 2;

    assert(find_max_satisfiable_prefix(&formula1) == 2);

    int pool2[10];
    pool2[0] = lit_neg(pizza); pool2[1] = lit_pos(sushi); pool2[2] = lit_neg(risotto);
    pool2[3] = lit_pos(pizza); pool2[4] = lit_neg(sushi); pool2[5] = lit_pos(risotto);
    pool2[6] = lit_neg(risotto);
    pool2[7] = lit_neg(sushi);
    pool2[8] = lit_pos(risotto);
    pool2[9] = lit_pos(sushi);
    Clause clauses2[] = {
        {0, 3, false},
        {3, 3, false},
        {6, 1, false},
        {7, 1, false},
        {8, 1, false},
        {9, 1, false}
    };
    Formula formula2;
    formula2.variable_count = 4;
    formula2.literal_pool = pool2;
    formula2.literal_pool_size = 10;
    formula2.clauses = clauses2;
    formula2.clause_count = 6;

    assert(find_max_satisfiable_prefix(&formula2) == 4);

    Formula empty_formula;
    empty_formula.variable_count = 0;
    empty_formula.literal_pool = NULL;
    empty_formula.literal_pool_size = 0;
    empty_formula.clauses = NULL;
    empty_formula.clause_count = 0;

    assert(find_max_satisfiable_prefix(&empty_formula) == 0);

    printf("test_threshold_search: OK\n");
    return 0;
}
