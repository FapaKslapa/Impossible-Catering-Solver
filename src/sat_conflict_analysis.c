#include "sat_internal.h"

int sat_analyze_conflict(SatSolver *solver, int conflicting_clause, int *out_literals, int *out_backjump_level) {
    solver->seen_epoch++;
    int out_size = 0;
    int count_at_current_level = 0;
    int clause_to_resolve = conflicting_clause;
    int trail_index = solver->trail_size - 1;
    int pivot_variable;

    for (;;) {
        int size = sat_clause_size(solver, clause_to_resolve);
        for (int i = 0; i < size; i++) {
            int literal = sat_clause_literal(solver, clause_to_resolve, i);
            int variable = lit_variable(literal);
            if (solver->seen[variable] == solver->seen_epoch) {
                continue;
            }
            solver->seen[variable] = solver->seen_epoch;
            solver->activity[variable] += solver->activity_increment;
            if (solver->decision_level[variable] == solver->current_decision_level) {
                count_at_current_level++;
            } else {
                out_literals[1 + out_size] = literal;
                out_size++;
            }
        }

        while (solver->seen[lit_variable(solver->trail[trail_index])] != solver->seen_epoch) {
            trail_index--;
        }
        pivot_variable = lit_variable(solver->trail[trail_index]);
        trail_index--;
        count_at_current_level--;

        if (count_at_current_level == 0) {
            break;
        }

        clause_to_resolve = solver->antecedent[pivot_variable];
        if (clause_to_resolve >= solver->formula->clause_count) {
            solver->learned_clauses[clause_to_resolve - solver->formula->clause_count].activity += solver->clause_activity_increment;
        }
    }

    out_literals[0] = lit_negate(solver->trail[trail_index + 1]);

    int backjump_level = 0;
    for (int i = 1; i < 1 + out_size; i++) {
        int level = solver->decision_level[lit_variable(out_literals[i])];
        if (level > backjump_level) {
            backjump_level = level;
        }
    }
    *out_backjump_level = backjump_level;

    solver->activity_increment *= 1.0 / 0.95;
    if (solver->activity_increment > 1e100) {
        for (int i = 0; i < solver->formula->variable_count; i++) {
            solver->activity[i] *= 1e-100;
        }
        solver->activity_increment *= 1e-100;
    }
    solver->clause_activity_increment *= 1.0 / 0.95;
    if (solver->clause_activity_increment > 1e100) {
        for (int i = 0; i < solver->learned_clause_count; i++) {
            solver->learned_clauses[i].activity *= 1e-100;
        }
        solver->clause_activity_increment *= 1e-100;
    }

    return 1 + out_size;
}
