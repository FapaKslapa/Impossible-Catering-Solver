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

static void test_reduce_self_overlapping_copy(void) {
    Formula formula = empty_formula(20);
    SatSolver *solver = sat_solver_create(&formula);
    solver->learned_clause_count = 0;
    solver->learned_literal_pool_size = 0;
    for (int i = 0; i < 20; i++) {
        solver->watch_head[2 * i] = -1;
        solver->watch_head[2 * i + 1] = -1;
        solver->decision_level[i] = -1;
        solver->antecedent[i] = -1;
        solver->assignment[i] = VALUE_UNASSIGNED;
    }

    int clause0[] = {lit_pos(0)};
    int clause1[] = {lit_pos(1), lit_neg(2), lit_pos(3), lit_neg(4), lit_pos(5), lit_neg(6), lit_pos(7), lit_neg(8)};
    int clause2[] = {lit_neg(9)};
    int clause3[] = {lit_pos(10), lit_neg(11), lit_pos(12), lit_neg(13), lit_pos(14), lit_neg(15), lit_pos(16), lit_neg(17)};

    sat_add_learned_clause(solver, clause0, 1);
    int index1 = sat_add_learned_clause(solver, clause1, 8);
    sat_add_learned_clause(solver, clause2, 1);
    int index3 = sat_add_learned_clause(solver, clause3, 8);

    solver->assignment[18] = VALUE_TRUE;
    solver->antecedent[18] = index1;
    solver->assignment[19] = VALUE_TRUE;
    solver->antecedent[19] = index3;

    sat_reduce_learned_clauses(solver);

    assert(solver->learned_clause_count == 2);

    int new_index1 = solver->antecedent[18];
    int new_index3 = solver->antecedent[19];

    assert(new_index1 >= formula.clause_count);
    assert(new_index3 >= formula.clause_count);

    assert(sat_clause_size(solver, new_index1) == 8);
    assert(sat_clause_literal(solver, new_index1, 0) == lit_pos(1));
    assert(sat_clause_literal(solver, new_index1, 1) == lit_neg(2));
    assert(sat_clause_literal(solver, new_index1, 2) == lit_pos(3));
    assert(sat_clause_literal(solver, new_index1, 3) == lit_neg(4));
    assert(sat_clause_literal(solver, new_index1, 4) == lit_pos(5));
    assert(sat_clause_literal(solver, new_index1, 5) == lit_neg(6));
    assert(sat_clause_literal(solver, new_index1, 6) == lit_pos(7));
    assert(sat_clause_literal(solver, new_index1, 7) == lit_neg(8));

    assert(sat_clause_size(solver, new_index3) == 8);
    assert(sat_clause_literal(solver, new_index3, 0) == lit_pos(10));
    assert(sat_clause_literal(solver, new_index3, 1) == lit_neg(11));
    assert(sat_clause_literal(solver, new_index3, 2) == lit_pos(12));
    assert(sat_clause_literal(solver, new_index3, 3) == lit_neg(13));
    assert(sat_clause_literal(solver, new_index3, 4) == lit_pos(14));
    assert(sat_clause_literal(solver, new_index3, 5) == lit_neg(15));
    assert(sat_clause_literal(solver, new_index3, 6) == lit_pos(16));
    assert(sat_clause_literal(solver, new_index3, 7) == lit_neg(17));

    sat_solver_destroy(solver);
}

static void test_add_learned_clause_fails_when_fully_locked(void) {
    int variable_count = SAT_MAX_LEARNED_CLAUSES;
    Formula formula = empty_formula(variable_count);
    SatSolver *solver = sat_solver_create(&formula);
    solver->learned_clause_count = 0;
    solver->learned_literal_pool_size = 0;
    for (int i = 0; i < variable_count; i++) {
        solver->watch_head[2 * i] = -1;
        solver->watch_head[2 * i + 1] = -1;
        solver->decision_level[i] = -1;
        solver->antecedent[i] = -1;
        solver->assignment[i] = VALUE_UNASSIGNED;
    }

    for (int i = 0; i < SAT_MAX_LEARNED_CLAUSES; i++) {
        int clause[] = {lit_pos(i)};
        int index = sat_add_learned_clause(solver, clause, 1);
        solver->assignment[i] = VALUE_TRUE;
        solver->antecedent[i] = index;
    }

    assert(solver->learned_clause_count == SAT_MAX_LEARNED_CLAUSES);

    int count_before = solver->learned_clause_count;
    int pool_size_before = solver->learned_literal_pool_size;

    int overflow_clause[] = {lit_pos(0)};
    int result = sat_add_learned_clause(solver, overflow_clause, 1);

    assert(result == -1);
    assert(solver->learned_clause_count == count_before);
    assert(solver->learned_literal_pool_size == pool_size_before);

    sat_solver_destroy(solver);
}

int main(void) {
    test_add_and_read_back();
    test_reduce_preserves_locked_clause();
    test_reduce_preserves_content_after_compaction();
    test_reduce_self_overlapping_copy();
    test_add_learned_clause_fails_when_fully_locked();
    printf("test_sat_clause_db: OK\n");
    return 0;
}
