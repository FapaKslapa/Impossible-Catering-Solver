#include "sat_solver.h"
#include "formula.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    int pizza = 0, sushi = 1, risotto = 2, torta = 3;
    (void) torta;

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

    SatSolver *solver1 = sat_solver_create(&formula1);
    assert(sat_solver_test_prefix(solver1, 2));
    sat_solver_destroy(solver1);

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

    SatSolver *solver2 = sat_solver_create(&formula2);
    assert(sat_solver_test_prefix(solver2, 4));
    assert(!sat_solver_test_prefix(solver2, 5));
    assert(!sat_solver_test_prefix(solver2, 6));
    sat_solver_destroy(solver2);

    int pool3[6];
    pool3[0] = lit_neg(pizza); pool3[1] = lit_pos(sushi);
    pool3[2] = lit_neg(pizza); pool3[3] = lit_neg(sushi);
    pool3[4] = lit_pos(pizza); pool3[5] = lit_pos(sushi);
    Clause clauses3[] = {
        {0, 2, false},
        {2, 2, false},
        {4, 2, false}
    };
    Formula formula3;
    formula3.variable_count = 4;
    formula3.literal_pool = pool3;
    formula3.literal_pool_size = 6;
    formula3.clauses = clauses3;
    formula3.clause_count = 3;

    SatSolver *solver3 = sat_solver_create(&formula3);
    assert(sat_solver_test_prefix(solver3, 3));
    sat_solver_destroy(solver3);

    int pool4[8];
    pool4[0] = lit_pos(pizza); pool4[1] = lit_pos(sushi);
    pool4[2] = lit_pos(pizza); pool4[3] = lit_neg(sushi);
    pool4[4] = lit_neg(pizza); pool4[5] = lit_pos(sushi);
    pool4[6] = lit_neg(pizza); pool4[7] = lit_neg(sushi);
    Clause clauses4[] = {
        {0, 2, false},
        {2, 2, false},
        {4, 2, false},
        {6, 2, false}
    };
    Formula formula4;
    formula4.variable_count = 4;
    formula4.literal_pool = pool4;
    formula4.literal_pool_size = 8;
    formula4.clauses = clauses4;
    formula4.clause_count = 4;

    SatSolver *solver4 = sat_solver_create(&formula4);
    assert(sat_solver_test_prefix(solver4, 3));
    assert(!sat_solver_test_prefix(solver4, 4));
    sat_solver_destroy(solver4);

    printf("test_sat_solver: OK\n");
    return 0;
}
