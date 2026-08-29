#ifndef SAT_CONFLICT_ANALYSIS_H
#define SAT_CONFLICT_ANALYSIS_H

#include "sat_solver.h"

int sat_analyze_conflict(SatSolver *solver, int conflicting_clause, int *out_literals, int *out_backjump_level);

#endif
