#ifndef SAT_INTERNAL_H
#define SAT_INTERNAL_H

#include "sat_solver.h"
#include "sat_restart.h"

#define SAT_LEARNED_POOL_CAPACITY 16000
#define SAT_MAX_LEARNED_CLAUSES 2000

typedef enum {
    VALUE_UNASSIGNED = 0,
    VALUE_TRUE = 1,
    VALUE_FALSE = -1
} VariableValue;

typedef struct {
    int offset;
    int size;
    double activity;
} LearnedClause;

typedef struct {
    int index;
    double activity;
} ActivityEntry;

struct SatSolver {
    const Formula *formula;
    int prefix_length;

    VariableValue *assignment;
    int *decision_level;
    int *antecedent;

    int *trail;
    int trail_size;

    int current_decision_level;

    int *watch_head;
    int *next_watch0;
    int *next_watch1;
    int *watched_literal0;
    int *watched_literal1;

    int *propagation_queue;
    int propagation_queue_head;
    int propagation_queue_tail;

    int *relevant_variables;
    int relevant_variable_count;

    double *activity;
    double activity_increment;
    VariableValue *saved_phase;

    int *seen;
    int seen_epoch;
    int *conflict_scratch;

    int *learned_literal_pool;
    int learned_literal_pool_size;
    int learned_literal_pool_capacity;
    LearnedClause *learned_clauses;
    int learned_clause_count;
    int learned_clause_capacity;
    double clause_activity_increment;
    int *reduce_survivor_indices;
    bool *reduce_survives;
    ActivityEntry *reduce_unlocked;
    int *reduce_remap;

    long long conflicts_since_restart;
    long long restart_index;
    long long restart_limit;
};

static inline int sat_clause_size(const SatSolver *solver, int clause_index) {
    if (clause_index < solver->formula->clause_count) {
        return solver->formula->clauses[clause_index].size;
    }
    return solver->learned_clauses[clause_index - solver->formula->clause_count].size;
}

static inline int sat_clause_literal(const SatSolver *solver, int clause_index, int position) {
    if (clause_index < solver->formula->clause_count) {
        const Clause *clause = &solver->formula->clauses[clause_index];
        return solver->formula->literal_pool[clause->offset + position];
    }
    const LearnedClause *clause = &solver->learned_clauses[clause_index - solver->formula->clause_count];
    return solver->learned_literal_pool[clause->offset + position];
}

bool sat_enqueue(SatSolver *solver, int literal, int antecedent_clause);
bool sat_propagate(SatSolver *solver, int *out_conflict_clause);
bool sat_solver_reset_and_setup(SatSolver *solver, int prefix_length);
void undo_to_level(SatSolver *solver, int target_level);

void sat_watch_clause(SatSolver *solver, int clause_index, int literal_a, int literal_b);
void sat_unwatch_clause(SatSolver *solver, int clause_index);

int sat_add_learned_clause(SatSolver *solver, const int *literals, int size);
void sat_reduce_learned_clauses(SatSolver *solver);

int sat_analyze_conflict(SatSolver *solver, int conflicting_clause, int *out_literals, int *out_backjump_level);

#endif
