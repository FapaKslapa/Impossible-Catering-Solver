#ifndef FORMULA_H
#define FORMULA_H

#include <stdbool.h>

typedef struct {
    int offset;
    int size;
    bool is_tautology;
} Clause;

typedef struct {
    int *literal_pool;
    int literal_pool_size;
    Clause *clauses;
    int clause_count;
    int variable_count;
} Formula;

void formula_destroy(Formula *formula);

static inline int lit_pos(int variable) { return variable * 2; }
static inline int lit_neg(int variable) { return variable * 2 + 1; }
static inline int lit_negate(int literal) { return literal ^ 1; }
static inline int lit_variable(int literal) { return literal >> 1; }

#endif
