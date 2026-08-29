#include "sat_internal.h"
#include <stdlib.h>

static size_t size_at_least_one(int n) {
    return (size_t) (n > 0 ? n : 1);
}

SatSolver *sat_solver_create(const Formula *formula) {
    SatSolver *solver = malloc(sizeof(SatSolver));
    solver->formula = formula;

    int variable_count = formula->variable_count;
    int clause_count = formula->clause_count;
    int literal_count = variable_count * 2;

    solver->assignment = malloc(size_at_least_one(variable_count) * sizeof(VariableValue));
    solver->trail = malloc(size_at_least_one(variable_count) * sizeof(int));
    solver->decisions = malloc(size_at_least_one(variable_count) * sizeof(Decision));
    solver->watch_head = malloc(size_at_least_one(literal_count) * sizeof(int));
    solver->next_watch0 = malloc(size_at_least_one(clause_count) * sizeof(int));
    solver->next_watch1 = malloc(size_at_least_one(clause_count) * sizeof(int));
    solver->watched_literal0 = malloc(size_at_least_one(clause_count) * sizeof(int));
    solver->watched_literal1 = malloc(size_at_least_one(clause_count) * sizeof(int));
    solver->propagation_queue = malloc(size_at_least_one(variable_count) * sizeof(int));
    solver->relevant_variables = malloc(size_at_least_one(variable_count) * sizeof(int));

    return solver;
}

void sat_solver_destroy(SatSolver *solver) {
    free(solver->assignment);
    free(solver->trail);
    free(solver->decisions);
    free(solver->watch_head);
    free(solver->next_watch0);
    free(solver->next_watch1);
    free(solver->watched_literal0);
    free(solver->watched_literal1);
    free(solver->propagation_queue);
    free(solver->relevant_variables);
    free(solver);
}

static void reset_solver_state(SatSolver *solver, int prefix_length) {
    int variable_count = solver->formula->variable_count;
    int literal_count = variable_count * 2;

    solver->prefix_length = prefix_length;
    for (int i = 0; i < variable_count; i++) {
        solver->assignment[i] = VALUE_UNASSIGNED;
    }
    for (int i = 0; i < literal_count; i++) {
        solver->watch_head[i] = -1;
    }
    solver->trail_size = 0;
    solver->decisions_size = 0;
    solver->propagation_queue_head = 0;
    solver->propagation_queue_tail = 0;
    solver->relevant_variable_count = 0;
}

static bool setup_clauses(SatSolver *solver, bool *has_positive, bool *has_negative) {
    const Formula *formula = solver->formula;
    bool *seen = calloc(size_at_least_one(formula->variable_count), sizeof(bool));

    bool ok = true;
    for (int clause_index = 0; clause_index < solver->prefix_length; clause_index++) {
        const Clause *clause = &formula->clauses[clause_index];
        if (clause->is_tautology) {
            continue;
        }
        if (clause->size == 0) {
            ok = false;
            break;
        }

        for (int i = 0; i < clause->size; i++) {
            int literal = formula->literal_pool[clause->offset + i];
            int variable = lit_variable(literal);
            if (!seen[variable]) {
                seen[variable] = true;
                solver->relevant_variables[solver->relevant_variable_count] = variable;
                solver->relevant_variable_count++;
            }
            if ((literal & 1) != 0) {
                has_negative[variable] = true;
            } else {
                has_positive[variable] = true;
            }
        }

        if (clause->size == 1) {
            if (!sat_enqueue(solver, formula->literal_pool[clause->offset])) {
                ok = false;
                break;
            }
            continue;
        }

        int literal0 = formula->literal_pool[clause->offset];
        int literal1 = formula->literal_pool[clause->offset + 1];
        solver->watched_literal0[clause_index] = literal0;
        solver->watched_literal1[clause_index] = literal1;
        solver->next_watch0[clause_index] = solver->watch_head[literal0];
        solver->watch_head[literal0] = clause_index;
        solver->next_watch1[clause_index] = solver->watch_head[literal1];
        solver->watch_head[literal1] = clause_index;
    }

    free(seen);
    return ok;
}

static bool apply_pure_literals(SatSolver *solver, const bool *has_positive, const bool *has_negative) {
    for (int i = 0; i < solver->relevant_variable_count; i++) {
        int variable = solver->relevant_variables[i];
        if (solver->assignment[variable] != VALUE_UNASSIGNED) {
            continue;
        }
        bool positive = has_positive[variable];
        bool negative = has_negative[variable];
        if (positive && !negative) {
            if (!sat_enqueue(solver, lit_pos(variable))) {
                return false;
            }
        } else if (negative && !positive) {
            if (!sat_enqueue(solver, lit_neg(variable))) {
                return false;
            }
        }
    }
    return true;
}

bool sat_solver_reset_and_setup(SatSolver *solver, int prefix_length) {
    reset_solver_state(solver, prefix_length);

    int variable_count = solver->formula->variable_count;
    bool *has_positive = calloc(size_at_least_one(variable_count), sizeof(bool));
    bool *has_negative = calloc(size_at_least_one(variable_count), sizeof(bool));

    bool ok = setup_clauses(solver, has_positive, has_negative);
    if (ok) {
        ok = apply_pure_literals(solver, has_positive, has_negative);
    }

    free(has_positive);
    free(has_negative);
    return ok;
}

static int pick_unassigned_variable(SatSolver *solver) {
    for (int i = 0; i < solver->relevant_variable_count; i++) {
        int variable = solver->relevant_variables[i];
        if (solver->assignment[variable] == VALUE_UNASSIGNED) {
            return variable;
        }
    }
    return -1;
}

static void undo_to_trail_size(SatSolver *solver, int target_trail_size) {
    while (solver->trail_size > target_trail_size) {
        solver->trail_size--;
        int literal = solver->trail[solver->trail_size];
        solver->assignment[lit_variable(literal)] = VALUE_UNASSIGNED;
    }
    solver->propagation_queue_head = solver->trail_size;
    solver->propagation_queue_tail = solver->trail_size;
}

static bool sat_search(SatSolver *solver) {
    for (;;) {
        bool no_conflict = sat_propagate(solver);
        if (!no_conflict) {
            while (solver->decisions_size > 0 &&
                   solver->decisions[solver->decisions_size - 1].tried_false) {
                solver->decisions_size--;
            }
            if (solver->decisions_size == 0) {
                return false;
            }
            Decision *decision = &solver->decisions[solver->decisions_size - 1];
            undo_to_trail_size(solver, decision->trail_position);
            decision->tried_false = true;
            sat_enqueue(solver, lit_neg(decision->variable));
            continue;
        }

        int variable = pick_unassigned_variable(solver);
        if (variable == -1) {
            return true;
        }

        solver->decisions[solver->decisions_size].variable = variable;
        solver->decisions[solver->decisions_size].tried_false = false;
        solver->decisions[solver->decisions_size].trail_position = solver->trail_size;
        solver->decisions_size++;

        sat_enqueue(solver, lit_pos(variable));
    }
}

bool sat_solver_test_prefix(SatSolver *solver, int prefix_length) {
    if (!sat_solver_reset_and_setup(solver, prefix_length)) {
        return false;
    }
    return sat_search(solver);
}
