#include "sat_clause_db.h"
#include "sat_internal.h"
#include "formula.h"
#include <assert.h>
#include <stdio.h>

static Formula empty_formula(int variable_count) {
    Formula formula;
    formula.variable_count = variable_count;
    formula.literal_pool = NULL;
    formula.literal_pool_size = 0;
    formula.clauses = NULL;
    formula.clause_count = 0;
    return formula;
}

static void test_add_and_read_back(void) {
    Formula formula = empty_formula(4);
    SatSolver *solver = sat_solver_create(&formula);
    solver->learned_clause_count = 0;
    solver->learned_literal_pool_size = 0;
    for (int i = 0; i < 4; i++) {
        solver->watch_head[2 * i] = -1;
        solver->watch_head[2 * i + 1] = -1;
        solver->decision_level[i] = -1;
        solver->antecedent[i] = -1;
    }

    int literals[] = {lit_pos(0), lit_neg(1), lit_pos(2)};
    int clause_index = sat_add_learned_clause(solver, literals, 3);

    assert(clause_index == formula.clause_count);
    assert(solver->learned_clause_count == 1);
    assert(sat_clause_size(solver, clause_index) == 3);
    assert(sat_clause_literal(solver, clause_index, 0) == lit_pos(0));
    assert(sat_clause_literal(solver, clause_index, 1) == lit_neg(1));
    assert(sat_clause_literal(solver, clause_index, 2) == lit_pos(2));
    assert(solver->watched_literal0[clause_index] == lit_pos(0));

    sat_solver_destroy(solver);
}

static void test_reduce_preserves_locked_clause(void) {
    Formula formula = empty_formula(6);
    SatSolver *solver = sat_solver_create(&formula);
    solver->learned_clause_count = 0;
    solver->learned_literal_pool_size = 0;
    for (int i = 0; i < 6; i++) {
        solver->watch_head[2 * i] = -1;
        solver->watch_head[2 * i + 1] = -1;
        solver->decision_level[i] = -1;
        solver->antecedent[i] = -1;
    }

    int low_activity[] = {lit_pos(0), lit_pos(1)};
    int locked[] = {lit_pos(2), lit_pos(3)};
    int high_activity[] = {lit_pos(4), lit_pos(5)};

    int low_index = sat_add_learned_clause(solver, low_activity, 2);
    int locked_index = sat_add_learned_clause(solver, locked, 2);
    int high_index = sat_add_learned_clause(solver, high_activity, 2);

    solver->learned_clauses[low_index - formula.clause_count].activity = 1.0;
    solver->learned_clauses[locked_index - formula.clause_count].activity = 1.0;
    solver->learned_clauses[high_index - formula.clause_count].activity = 100.0;

    solver->assignment[2] = VALUE_TRUE;
    solver->antecedent[2] = locked_index;

    sat_reduce_learned_clauses(solver);

    bool locked_survived = false;
    bool high_survived = false;
    for (int i = 0; i < solver->learned_clause_count; i++) {
        int global_index = formula.clause_count + i;
        if (global_index == locked_index || solver->antecedent[2] == global_index) {
            locked_survived = true;
        }
    }
    assert(solver->antecedent[2] >= formula.clause_count);
    for (int i = 0; i < solver->learned_clause_count; i++) {
        if (solver->learned_clauses[i].activity == 100.0) {
            high_survived = true;
        }
    }
    assert(high_survived);
    assert(locked_survived);
    assert(solver->learned_clause_count < 3);

    sat_solver_destroy(solver);
}

static void test_reduce_preserves_content_after_compaction(void) {
    Formula formula = empty_formula(12);
    SatSolver *solver = sat_solver_create(&formula);
    solver->learned_clause_count = 0;
    solver->learned_literal_pool_size = 0;
    for (int i = 0; i < 12; i++) {
        solver->watch_head[2 * i] = -1;
        solver->watch_head[2 * i + 1] = -1;
        solver->decision_level[i] = -1;
        solver->antecedent[i] = -1;
        solver->assignment[i] = VALUE_UNASSIGNED;
    }

    int clause0[] = {lit_pos(0), lit_neg(1)};
    int clause1[] = {lit_pos(2), lit_neg(3), lit_pos(4)};
    int clause2[] = {lit_neg(5)};
    int clause3[] = {lit_pos(6), lit_neg(7), lit_pos(8), lit_neg(9)};
    int clause4[] = {lit_pos(10), lit_neg(11)};
    int clause5[] = {lit_neg(0), lit_pos(3), lit_neg(6)};

    int index0 = sat_add_learned_clause(solver, clause0, 2);
    sat_add_learned_clause(solver, clause1, 3);
    int index2 = sat_add_learned_clause(solver, clause2, 1);
    sat_add_learned_clause(solver, clause3, 4);
    sat_add_learned_clause(solver, clause4, 2);
    int index5 = sat_add_learned_clause(solver, clause5, 3);

    solver->assignment[1] = VALUE_TRUE;
    solver->antecedent[1] = index0;
    solver->assignment[5] = VALUE_TRUE;
    solver->antecedent[5] = index2;
    solver->assignment[11] = VALUE_TRUE;
    solver->antecedent[11] = index5;

    sat_reduce_learned_clauses(solver);

    assert(solver->learned_clause_count == 3);

    int new_index0 = solver->antecedent[1];
    int new_index2 = solver->antecedent[5];
    int new_index5 = solver->antecedent[11];

    assert(new_index0 >= formula.clause_count);
    assert(new_index2 >= formula.clause_count);
    assert(new_index5 >= formula.clause_count);

    assert(sat_clause_size(solver, new_index0) == 2);
    assert(sat_clause_literal(solver, new_index0, 0) == lit_pos(0));
    assert(sat_clause_literal(solver, new_index0, 1) == lit_neg(1));

    assert(sat_clause_size(solver, new_index2) == 1);
    assert(sat_clause_literal(solver, new_index2, 0) == lit_neg(5));

    assert(sat_clause_size(solver, new_index5) == 3);
    assert(sat_clause_literal(solver, new_index5, 0) == lit_neg(0));
    assert(sat_clause_literal(solver, new_index5, 1) == lit_pos(3));
    assert(sat_clause_literal(solver, new_index5, 2) == lit_neg(6));

    sat_solver_destroy(solver);
}

int main(void) {
    test_add_and_read_back();
    test_reduce_preserves_locked_clause();
    test_reduce_preserves_content_after_compaction();
    printf("test_sat_clause_db: OK\n");
    return 0;
}
