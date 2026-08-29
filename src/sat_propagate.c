#include "sat_internal.h"

static int other_watch_literal(SatSolver *solver, int clause_index, int literal) {
    if (solver->watched_literal0[clause_index] == literal) {
        return solver->watched_literal1[clause_index];
    }
    return solver->watched_literal0[clause_index];
}

static void set_watch_literal(SatSolver *solver, int clause_index, int old_literal, int new_literal) {
    if (solver->watched_literal0[clause_index] == old_literal) {
        solver->watched_literal0[clause_index] = new_literal;
    } else {
        solver->watched_literal1[clause_index] = new_literal;
    }
}

static int *next_pointer_for_slot(SatSolver *solver, int clause_index, int literal) {
    if (solver->watched_literal0[clause_index] == literal) {
        return &solver->next_watch0[clause_index];
    }
    return &solver->next_watch1[clause_index];
}

static VariableValue literal_value(const SatSolver *solver, int literal) {
    VariableValue variable_value = solver->assignment[lit_variable(literal)];
    if (variable_value == VALUE_UNASSIGNED) {
        return VALUE_UNASSIGNED;
    }
    bool literal_is_negative = (literal & 1) != 0;
    bool variable_is_true = (variable_value == VALUE_TRUE);
    bool literal_is_true = literal_is_negative ? !variable_is_true : variable_is_true;
    return literal_is_true ? VALUE_TRUE : VALUE_FALSE;
}

static bool find_replacement_literal(SatSolver *solver, int clause_index, int avoid_literal_a, int avoid_literal_b, int *out_literal) {
    int size = sat_clause_size(solver, clause_index);
    for (int i = 0; i < size; i++) {
        int literal = sat_clause_literal(solver, clause_index, i);
        if (literal == avoid_literal_a || literal == avoid_literal_b) {
            continue;
        }
        if (literal_value(solver, literal) != VALUE_FALSE) {
            *out_literal = literal;
            return true;
        }
    }
    return false;
}

void sat_watch_clause(SatSolver *solver, int clause_index, int literal_a, int literal_b) {
    solver->watched_literal0[clause_index] = literal_a;
    solver->watched_literal1[clause_index] = literal_b;
    solver->next_watch0[clause_index] = solver->watch_head[literal_a];
    solver->watch_head[literal_a] = clause_index;
    solver->next_watch1[clause_index] = solver->watch_head[literal_b];
    solver->watch_head[literal_b] = clause_index;
}

static void unlink_clause_from_watch_list(SatSolver *solver, int clause_index, int literal) {
    int previous_clause = -1;
    int current_clause = solver->watch_head[literal];
    while (current_clause != clause_index) {
        previous_clause = current_clause;
        current_clause = *next_pointer_for_slot(solver, current_clause, literal);
    }
    int next_clause = *next_pointer_for_slot(solver, clause_index, literal);
    if (previous_clause == -1) {
        solver->watch_head[literal] = next_clause;
    } else {
        *next_pointer_for_slot(solver, previous_clause, literal) = next_clause;
    }
}

void sat_unwatch_clause(SatSolver *solver, int clause_index) {
    unlink_clause_from_watch_list(solver, clause_index, solver->watched_literal0[clause_index]);
    unlink_clause_from_watch_list(solver, clause_index, solver->watched_literal1[clause_index]);
}

bool sat_enqueue(SatSolver *solver, int literal, int antecedent_clause) {
    int variable = lit_variable(literal);
    bool literal_is_negative = (literal & 1) != 0;
    VariableValue wanted = literal_is_negative ? VALUE_FALSE : VALUE_TRUE;
    VariableValue current = solver->assignment[variable];
    if (current == wanted) {
        return true;
    }
    if (current != VALUE_UNASSIGNED) {
        return false;
    }
    solver->assignment[variable] = wanted;
    solver->decision_level[variable] = solver->current_decision_level;
    solver->antecedent[variable] = antecedent_clause;
    solver->trail[solver->trail_size] = literal;
    solver->trail_size++;
    solver->propagation_queue[solver->propagation_queue_tail] = literal;
    solver->propagation_queue_tail++;
    return true;
}

bool sat_propagate(SatSolver *solver, int *out_conflict_clause) {
    while (solver->propagation_queue_head < solver->propagation_queue_tail) {
        int true_literal = solver->propagation_queue[solver->propagation_queue_head];
        solver->propagation_queue_head++;

        int falsified_literal = lit_negate(true_literal);
        int previous_clause = -1;
        int current_clause = solver->watch_head[falsified_literal];

        while (current_clause != -1) {
            int *next_slot = next_pointer_for_slot(solver, current_clause, falsified_literal);
            int next_clause = *next_slot;

            int other_literal = other_watch_literal(solver, current_clause, falsified_literal);
            VariableValue other_value = literal_value(solver, other_literal);

            if (other_value == VALUE_TRUE) {
                previous_clause = current_clause;
                current_clause = next_clause;
                continue;
            }

            int replacement_literal;
            if (find_replacement_literal(solver, current_clause, falsified_literal, other_literal, &replacement_literal)) {
                if (previous_clause == -1) {
                    solver->watch_head[falsified_literal] = next_clause;
                } else {
                    *next_pointer_for_slot(solver, previous_clause, falsified_literal) = next_clause;
                }

                set_watch_literal(solver, current_clause, falsified_literal, replacement_literal);
                int *moved_slot = next_pointer_for_slot(solver, current_clause, replacement_literal);
                *moved_slot = solver->watch_head[replacement_literal];
                solver->watch_head[replacement_literal] = current_clause;

                current_clause = next_clause;
                continue;
            }

            if (other_value == VALUE_FALSE) {
                *out_conflict_clause = current_clause;
                return false;
            }

            if (!sat_enqueue(solver, other_literal, current_clause)) {
                *out_conflict_clause = current_clause;
                return false;
            }

            previous_clause = current_clause;
            current_clause = next_clause;
        }
    }
    return true;
}
