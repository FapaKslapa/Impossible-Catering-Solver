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
    solver->decision_level = malloc(size_at_least_one(variable_count) * sizeof(int));
    solver->antecedent = malloc(size_at_least_one(variable_count) * sizeof(int));
    solver->trail = malloc(size_at_least_one(variable_count) * sizeof(int));
    solver->watch_head = malloc(size_at_least_one(literal_count) * sizeof(int));
    solver->propagation_queue = malloc(size_at_least_one(variable_count) * sizeof(int));
    solver->relevant_variables = malloc(size_at_least_one(variable_count) * sizeof(int));
    solver->activity = malloc(size_at_least_one(variable_count) * sizeof(double));
    solver->saved_phase = malloc(size_at_least_one(variable_count) * sizeof(VariableValue));
    solver->seen = malloc(size_at_least_one(variable_count) * sizeof(int));
    solver->conflict_scratch = malloc(size_at_least_one(variable_count) * sizeof(int));

    int max_clause_count = clause_count + SAT_MAX_LEARNED_CLAUSES;
    solver->next_watch0 = malloc(size_at_least_one(max_clause_count) * sizeof(int));
    solver->next_watch1 = malloc(size_at_least_one(max_clause_count) * sizeof(int));
    solver->watched_literal0 = malloc(size_at_least_one(max_clause_count) * sizeof(int));
    solver->watched_literal1 = malloc(size_at_least_one(max_clause_count) * sizeof(int));

    solver->learned_literal_pool = malloc(SAT_LEARNED_POOL_CAPACITY * sizeof(int));
    solver->learned_literal_pool_capacity = SAT_LEARNED_POOL_CAPACITY;
    solver->learned_clauses = malloc(SAT_MAX_LEARNED_CLAUSES * sizeof(LearnedClause));
    solver->learned_clause_capacity = SAT_MAX_LEARNED_CLAUSES;

    return solver;
}

void sat_solver_destroy(SatSolver *solver) {
    free(solver->assignment);
    free(solver->decision_level);
    free(solver->antecedent);
    free(solver->trail);
    free(solver->watch_head);
    free(solver->next_watch0);
    free(solver->next_watch1);
    free(solver->watched_literal0);
    free(solver->watched_literal1);
    free(solver->propagation_queue);
    free(solver->relevant_variables);
    free(solver->activity);
    free(solver->saved_phase);
    free(solver->seen);
    free(solver->conflict_scratch);
    free(solver->learned_literal_pool);
    free(solver->learned_clauses);
    free(solver);
}

static void reset_solver_state(SatSolver *solver, int prefix_length) {
    int variable_count = solver->formula->variable_count;
    int literal_count = variable_count * 2;

    solver->prefix_length = prefix_length;
    for (int i = 0; i < variable_count; i++) {
        solver->assignment[i] = VALUE_UNASSIGNED;
        solver->decision_level[i] = -1;
        solver->antecedent[i] = -1;
        solver->activity[i] = 0.0;
        solver->saved_phase[i] = VALUE_TRUE;
        solver->seen[i] = 0;
    }
    for (int i = 0; i < literal_count; i++) {
        solver->watch_head[i] = -1;
    }
    solver->trail_size = 0;
    solver->current_decision_level = 0;
    solver->propagation_queue_head = 0;
    solver->propagation_queue_tail = 0;
    solver->relevant_variable_count = 0;
    solver->activity_increment = 1.0;
    solver->seen_epoch = 0;
    solver->learned_literal_pool_size = 0;
    solver->learned_clause_count = 0;
    solver->clause_activity_increment = 1.0;
    solver->conflicts_since_restart = 0;
    solver->restart_index = 0;
    solver->restart_limit = 100 * sat_luby(1);
}

static bool setup_clauses(SatSolver *solver) {
    const Formula *formula = solver->formula;
    bool *seen_relevant = calloc(size_at_least_one(formula->variable_count), sizeof(bool));

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
            if (!seen_relevant[variable]) {
                seen_relevant[variable] = true;
                solver->relevant_variables[solver->relevant_variable_count] = variable;
                solver->relevant_variable_count++;
            }
        }

        if (clause->size == 1) {
            if (!sat_enqueue(solver, formula->literal_pool[clause->offset], clause_index)) {
                ok = false;
                break;
            }
            continue;
        }

        int literal0 = formula->literal_pool[clause->offset];
        int literal1 = formula->literal_pool[clause->offset + 1];
        sat_watch_clause(solver, clause_index, literal0, literal1);
    }

    free(seen_relevant);
    return ok;
}

bool sat_solver_reset_and_setup(SatSolver *solver, int prefix_length) {
    reset_solver_state(solver, prefix_length);
    return setup_clauses(solver);
}

static int pick_unassigned_variable(SatSolver *solver) {
    int best_variable = -1;
    double best_activity = -1.0;
    for (int i = 0; i < solver->relevant_variable_count; i++) {
        int variable = solver->relevant_variables[i];
        if (solver->assignment[variable] != VALUE_UNASSIGNED) {
            continue;
        }
        if (best_variable == -1 || solver->activity[variable] > best_activity) {
            best_variable = variable;
            best_activity = solver->activity[variable];
        }
    }
    return best_variable;
}

void undo_to_level(SatSolver *solver, int target_level) {
    while (solver->trail_size > 0 &&
           solver->decision_level[lit_variable(solver->trail[solver->trail_size - 1])] > target_level) {
        solver->trail_size--;
        int literal = solver->trail[solver->trail_size];
        int variable = lit_variable(literal);
        solver->saved_phase[variable] = solver->assignment[variable];
        solver->assignment[variable] = VALUE_UNASSIGNED;
    }
    if (solver->propagation_queue_head > solver->trail_size) {
        solver->propagation_queue_head = solver->trail_size;
    }
    solver->propagation_queue_tail = solver->trail_size;
    solver->current_decision_level = target_level;
}

static bool sat_search(SatSolver *solver) {
    for (;;) {
        int conflict_clause;
        bool no_conflict = sat_propagate(solver, &conflict_clause);

        if (!no_conflict) {
            if (solver->current_decision_level == 0) {
                return false;
            }

            int backjump_level;
            int learned_size = sat_analyze_conflict(solver, conflict_clause, solver->conflict_scratch, &backjump_level);
            undo_to_level(solver, backjump_level);
            int learned_index = sat_add_learned_clause(solver, solver->conflict_scratch, learned_size);
            sat_enqueue(solver, solver->conflict_scratch[0], learned_index);

            solver->conflicts_since_restart++;
            if (solver->conflicts_since_restart >= solver->restart_limit) {
                solver->conflicts_since_restart = 0;
                solver->restart_index++;
                solver->restart_limit = 100 * sat_luby(solver->restart_index + 1);
                undo_to_level(solver, 0);
            }
            continue;
        }

        int variable = pick_unassigned_variable(solver);
        if (variable == -1) {
            return true;
        }

        solver->current_decision_level++;
        int literal = (solver->saved_phase[variable] == VALUE_FALSE) ? lit_neg(variable) : lit_pos(variable);
        sat_enqueue(solver, literal, -1);
    }
}

bool sat_solver_test_prefix(SatSolver *solver, int prefix_length) {
    if (!sat_solver_reset_and_setup(solver, prefix_length)) {
        return false;
    }
    return sat_search(solver);
}
