#include "sat_solver.h"
#include "sat_internal.h"
#include "formula.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    int a = 0, b = 1;

    int pool[2];
    pool[0] = lit_pos(a);
    pool[1] = lit_pos(b);

    Clause clauses[1];
    clauses[0].offset = 0;
    clauses[0].size = 1;
    clauses[0].is_tautology = false;

    Formula formula;
    formula.variable_count = 2;
    formula.literal_pool = pool;
    formula.literal_pool_size = 2;
    formula.clauses = clauses;
    formula.clause_count = 1;

    SatSolver *solver = sat_solver_create(&formula);
    sat_solver_reset_and_setup(solver, 0);

    solver->current_decision_level = 1;
    sat_enqueue(solver, lit_pos(a), -1);
    solver->propagation_queue_head = solver->trail_size;

    undo_to_level(solver, 0);

    sat_enqueue(solver, lit_pos(b), -1);
    int trail_index_of_b = solver->trail_size - 1;

    undo_to_level(solver, 0);

    assert(solver->propagation_queue_head <= trail_index_of_b);
    assert(solver->trail_size == trail_index_of_b + 1);
    assert(solver->assignment[b] == VALUE_TRUE);

    sat_solver_destroy(solver);
    printf("test_sat_undo_to_level: OK\n");
    return 0;
}
