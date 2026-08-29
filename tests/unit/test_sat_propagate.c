#include "sat_internal.h"
#include "formula.h"
#include <assert.h>
#include <stdio.h>

static Formula build_formula(int variable_count, int *literal_pool, int literal_pool_size,
                              Clause *clauses, int clause_count) {
    Formula formula;
    formula.variable_count = variable_count;
    formula.literal_pool = literal_pool;
    formula.literal_pool_size = literal_pool_size;
    formula.clauses = clauses;
    formula.clause_count = clause_count;
    return formula;
}

static void test_propagation_chain(void) {
    int a = 0, b = 1, c = 2;
    int pool[7];
    pool[0] = lit_pos(a); pool[1] = lit_pos(b);
    pool[2] = lit_neg(a); pool[3] = lit_pos(c);
    pool[4] = lit_neg(b); pool[5] = lit_neg(c);
    pool[6] = lit_neg(a);
    Clause clauses[] = {
        {0, 2, false},
        {2, 2, false},
        {4, 2, false},
        {6, 1, false}
    };
    Formula formula = build_formula(3, pool, 7, clauses, 4);

    SatSolver *solver = sat_solver_create(&formula);
    assert(sat_solver_reset_and_setup(solver, 4));
    int conflict_clause;
    assert(sat_propagate(solver, &conflict_clause));

    assert(solver->assignment[a] == VALUE_FALSE);
    assert(solver->assignment[b] == VALUE_TRUE);
    assert(solver->assignment[c] == VALUE_FALSE);

    sat_solver_destroy(solver);
}

static void test_propagation_conflict(void) {
    int a = 0, b = 1, c = 2;
    int pool[8];
    pool[0] = lit_pos(a); pool[1] = lit_pos(b);
    pool[2] = lit_neg(a); pool[3] = lit_pos(c);
    pool[4] = lit_neg(b); pool[5] = lit_neg(c);
    pool[6] = lit_neg(a);
    pool[7] = lit_pos(c);
    Clause clauses[] = {
        {0, 2, false},
        {2, 2, false},
        {4, 2, false},
        {6, 1, false},
        {7, 1, false}
    };
    Formula formula = build_formula(3, pool, 8, clauses, 5);

    SatSolver *solver = sat_solver_create(&formula);
    assert(sat_solver_reset_and_setup(solver, 5));
    int conflict_clause;
    assert(!sat_propagate(solver, &conflict_clause));
    assert(conflict_clause == 2);

    sat_solver_destroy(solver);
}

static void test_propagation_relocation(void) {
    int a = 0, b = 1, c = 2, d = 3;
    int pool[11];
    pool[0] = lit_pos(a); pool[1] = lit_pos(b); pool[2] = lit_pos(c);
    pool[3] = lit_pos(a); pool[4] = lit_pos(d);
    pool[5] = lit_neg(a);
    pool[6] = lit_neg(b); pool[7] = lit_neg(c); pool[8] = lit_neg(d);
    Clause clauses[] = {
        {0, 3, false},
        {3, 2, false},
        {5, 1, false},
        {6, 3, false}
    };
    Formula formula = build_formula(4, pool, 9, clauses, 4);

    SatSolver *solver = sat_solver_create(&formula);
    assert(sat_solver_reset_and_setup(solver, 4));
    int conflict_clause;
    assert(sat_propagate(solver, &conflict_clause));

    assert(solver->assignment[a] == VALUE_FALSE);
    assert(solver->assignment[d] == VALUE_TRUE);
    assert(solver->assignment[b] == VALUE_UNASSIGNED);
    assert(solver->assignment[c] == VALUE_UNASSIGNED);

    sat_solver_destroy(solver);
}

int main(void) {
    test_propagation_chain();
    test_propagation_conflict();
    test_propagation_relocation();
    printf("test_sat_propagate: OK\n");
    return 0;
}
