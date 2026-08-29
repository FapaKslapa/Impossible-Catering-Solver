#include "sat_internal.h"
#include <stdlib.h>
#include <string.h>

static int compare_int_ascending(const void *a, const void *b) {
    int left = *(const int *) a;
    int right = *(const int *) b;
    return (left > right) - (left < right);
}

typedef struct {
    int index;
    double activity;
} ActivityEntry;

static int compare_activity_descending(const void *a, const void *b) {
    double left = ((const ActivityEntry *) a)->activity;
    double right = ((const ActivityEntry *) b)->activity;
    return (left < right) - (left > right);
}

void sat_reduce_learned_clauses(SatSolver *solver) {
    int kept_count = 0;
    int survivor_indices[SAT_MAX_LEARNED_CLAUSES];
    // survives[i] means "learned clause i will still exist after this call
    // returns". It starts out true for exactly the locked clauses and is also
    // set for the activity-selected survivors at the point they are appended to
    // survivor_indices, so it stays in lockstep with survivor_indices[0..kept_count)
    // and can be used directly as the O(1) unwatch decision below.
    bool survives[SAT_MAX_LEARNED_CLAUSES];
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

    int unlocked_count = solver->learned_clause_count - kept_count;
    ActivityEntry *unlocked = malloc((unlocked_count > 0 ? unlocked_count : 1) * sizeof(ActivityEntry));
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
    free(unlocked);

    // The compaction loop below reuses global clause-index slots in place
    // (new_global_index = clause_count + k for k = 0, 1, ...), which is only
    // safe if survivors are visited in ascending original-index order: that
    // guarantees a slot is never overwritten before its own not-yet-processed
    // survivor content has been read and unwatched. Locked clauses are
    // appended in ascending order above, but the activity-based fill pass can
    // append a lower original index after a higher one, so sort explicitly.
    qsort(survivor_indices, kept_count, sizeof(int), compare_int_ascending);

    for (int i = 0; i < solver->learned_clause_count; i++) {
        if (!survives[i] && solver->learned_clauses[i].size > 1) {
            sat_unwatch_clause(solver, solver->formula->clause_count + i);
        }
    }

    int *remap = malloc((solver->learned_clause_count > 0 ? solver->learned_clause_count : 1) * sizeof(int));
    for (int i = 0; i < solver->learned_clause_count; i++) {
        remap[i] = -1;
    }
    for (int k = 0; k < kept_count; k++) {
        remap[survivor_indices[k]] = solver->formula->clause_count + k;
    }

    LearnedClause *new_clauses = malloc(solver->learned_clause_capacity * sizeof(LearnedClause));
    int *new_pool = malloc(solver->learned_literal_pool_capacity * sizeof(int));
    int new_pool_size = 0;

    for (int k = 0; k < kept_count; k++) {
        int old_index = survivor_indices[k];
        LearnedClause old_clause = solver->learned_clauses[old_index];
        int new_offset = new_pool_size;
        for (int i = 0; i < old_clause.size; i++) {
            new_pool[new_offset + i] = solver->learned_literal_pool[old_clause.offset + i];
        }
        new_pool_size += old_clause.size;

        new_clauses[k].offset = new_offset;
        new_clauses[k].size = old_clause.size;
        new_clauses[k].activity = old_clause.activity;

        int old_global_index = solver->formula->clause_count + old_index;
        int new_global_index = solver->formula->clause_count + k;
        if (old_clause.size > 1) {
            int watch0 = solver->watched_literal0[old_global_index];
            int watch1 = solver->watched_literal1[old_global_index];
            sat_unwatch_clause(solver, old_global_index);
            sat_watch_clause(solver, new_global_index, watch0, watch1);
        }
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
    free(remap);

    memcpy(solver->learned_clauses, new_clauses, kept_count * sizeof(LearnedClause));
    memcpy(solver->learned_literal_pool, new_pool, new_pool_size * sizeof(int));
    solver->learned_clause_count = kept_count;
    solver->learned_literal_pool_size = new_pool_size;

    free(new_clauses);
    free(new_pool);
}

int sat_add_learned_clause(SatSolver *solver, const int *literals, int size) {
    if (solver->learned_clause_count >= solver->learned_clause_capacity ||
        solver->learned_literal_pool_size + size > solver->learned_literal_pool_capacity) {
        sat_reduce_learned_clauses(solver);
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
