#ifndef SAT_CLAUSE_DB_H
#define SAT_CLAUSE_DB_H

#include "sat_solver.h"

int sat_add_learned_clause(SatSolver *solver, const int *literals, int size);
void sat_reduce_learned_clauses(SatSolver *solver);

#endif
