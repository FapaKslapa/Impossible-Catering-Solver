#include "sat_internal.h"
#include <stdlib.h>

static int compare_int_ascending(const void *a, const void *b) {
    int left = *(const int *) a;
    int right = *(const int *) b;
    return (left > right) - (left < right);
}

static int compare_activity_descending(const void *a, const void *b) {
    double left = ((const ActivityEntry *) a)->activity;
    double right = ((const ActivityEntry *) b)->activity;
    return (left < right) - (left > right);
}

void sat_reduce_learned_clauses(SatSolver *solver) {
    int kept_count = 0;
    int *survivor_indices = solver->reduce_survivor_indices;
    bool *survives = solver->reduce_survives;
    for (int i = 0; i < solver->learned_clause_count; i++) {
        survives[i] = false;
    }

    for (int v = 0; v < solver->formula->variable_count; v++) {
        if (solver->assignment[v] == VALUE_UNASSIGNED) {
            continue;
        }
        int antecedent_clause = solver->antecedent[v];
        if (antecedent_clause >= solver->formula->clause_count) {
            survives[antecedent_clause - solver->formula->clause_count] = true;
        }
    }

    for (int i = 0; i < solver->learned_clause_count; i++) {
        if (survives[i]) {
            survivor_indices[kept_count] = i;
            kept_count++;
        }
    }

    int target_survivor_count = (solver->learned_clause_count + 1) / 2;
    if (target_survivor_count < kept_count) {
        target_survivor_count = kept_count;
    }

    ActivityEntry *unlocked = solver->reduce_unlocked;
    int unlocked_size = 0;
    for (int i = 0; i < solver->learned_clause_count; i++) {
        if (!survives[i]) {
            unlocked[unlocked_size].index = i;
            unlocked[unlocked_size].activity = solver->learned_clauses[i].activity;
            unlocked_size++;
        }
    }
    qsort(unlocked, unlocked_size, sizeof(ActivityEntry), compare_activity_descending);

    int additional_needed = target_survivor_count - kept_count;
    for (int i = 0; i < additional_needed && i < unlocked_size; i++) {
        survivor_indices[kept_count] = unlocked[i].index;
        survives[unlocked[i].index] = true;
        kept_count++;
    }

    // The compaction loop below assumes survivor_indices[k] >= k for every k,
    // which requires this array to be sorted ascending before compaction runs.
    qsort(survivor_indices, kept_count, sizeof(int), compare_int_ascending);

    for (int i = 0; i < solver->learned_clause_count; i++) {
        if (!survives[i] && solver->learned_clauses[i].size > 1) {
            sat_unwatch_clause(solver, solver->formula->clause_count + i);
        }
    }

    int *remap = solver->reduce_remap;
    for (int i = 0; i < solver->learned_clause_count; i++) {
        remap[i] = -1;
    }
    for (int k = 0; k < kept_count; k++) {
        remap[survivor_indices[k]] = solver->formula->clause_count + k;
    }

    int new_pool_size = 0;
    for (int k = 0; k < kept_count; k++) {
        int old_index = survivor_indices[k];
        LearnedClause old_clause = solver->learned_clauses[old_index];
        int new_offset = new_pool_size;
        for (int i = 0; i < old_clause.size; i++) {
            solver->learned_literal_pool[new_offset + i] = solver->learned_literal_pool[old_clause.offset + i];
        }
        new_pool_size += old_clause.size;

        int old_global_index = solver->formula->clause_count + old_index;
        int new_global_index = solver->formula->clause_count + k;
        if (old_clause.size > 1) {
            int watch0 = solver->watched_literal0[old_global_index];
            int watch1 = solver->watched_literal1[old_global_index];
            sat_unwatch_clause(solver, old_global_index);
            sat_watch_clause(solver, new_global_index, watch0, watch1);
        }

        solver->learned_clauses[k].offset = new_offset;
        solver->learned_clauses[k].size = old_clause.size;
        solver->learned_clauses[k].activity = old_clause.activity;
    }

    for (int v = 0; v < solver->formula->variable_count; v++) {
        if (solver->assignment[v] == VALUE_UNASSIGNED) {
            continue;
        }
        int antecedent_clause = solver->antecedent[v];
        if (antecedent_clause >= solver->formula->clause_count) {
            solver->antecedent[v] = remap[antecedent_clause - solver->formula->clause_count];
        }
    }

    solver->learned_clause_count = kept_count;
    solver->learned_literal_pool_size = new_pool_size;
}

int sat_add_learned_clause(SatSolver *solver, const int *literals, int size) {
    while (solver->learned_clause_count >= solver->learned_clause_capacity ||
           solver->learned_literal_pool_size + size > solver->learned_literal_pool_capacity) {
        int count_before_reduce = solver->learned_clause_count;
        sat_reduce_learned_clauses(solver);
        if (solver->learned_clause_count >= count_before_reduce) {
            break;
        }
    }

    int offset = solver->learned_literal_pool_size;
    for (int i = 0; i < size; i++) {
        solver->learned_literal_pool[offset + i] = literals[i];
    }
    solver->learned_literal_pool_size += size;

    int clause_index = solver->formula->clause_count + solver->learned_clause_count;
    solver->learned_clauses[solver->learned_clause_count].offset = offset;
    solver->learned_clauses[solver->learned_clause_count].size = size;
    solver->learned_clauses[solver->learned_clause_count].activity = solver->clause_activity_increment;
    solver->learned_clause_count++;

    if (size < 2) {
        return clause_index;
    }

    int second_watch_position = 1;
    int best_level = solver->decision_level[lit_variable(literals[1])];
    for (int i = 2; i < size; i++) {
        int level = solver->decision_level[lit_variable(literals[i])];
        if (level > best_level) {
            best_level = level;
            second_watch_position = i;
        }
    }

    sat_watch_clause(solver, clause_index, literals[0], literals[second_watch_position]);
    return clause_index;
}
