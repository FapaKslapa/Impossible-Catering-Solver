#ifndef SAT_SOLVER_H
#define SAT_SOLVER_H

#include <stdbool.h>
#include "formula.h"

typedef struct SatSolver SatSolver;

SatSolver *sat_solver_create(const Formula *formula);
bool sat_solver_test_prefix(SatSolver *solver, int prefix_length);
void sat_solver_destroy(SatSolver *solver);

#endif
