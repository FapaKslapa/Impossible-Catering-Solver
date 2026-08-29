#ifndef SAT_INTERNAL_H
#define SAT_INTERNAL_H

#include "sat_solver.h"

typedef enum {
    VALUE_UNASSIGNED = 0,
    VALUE_TRUE = 1,
    VALUE_FALSE = -1
} VariableValue;

typedef struct {
    int variable;
    bool tried_false;
    int trail_position;
} Decision;

struct SatSolver {
    const Formula *formula;
    int prefix_length;

    VariableValue *assignment;

    int *trail;
    int trail_size;

    Decision *decisions;
    int decisions_size;

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
};

bool sat_enqueue(SatSolver *solver, int literal);
bool sat_propagate(SatSolver *solver);
bool sat_solver_reset_and_setup(SatSolver *solver, int prefix_length);

#endif
