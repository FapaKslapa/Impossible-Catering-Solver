#include "threshold_search.h"
#include "sat_solver.h"

int find_max_satisfiable_prefix(const Formula *formula) {
    SatSolver *solver = sat_solver_create(formula);

    int low = 0;
    int high = formula->clause_count;

    if (sat_solver_test_prefix(solver, high)) {
        sat_solver_destroy(solver);
        return high;
    }

    while (low < high) {
        int mid = low + (high - low + 1) / 2;
        if (sat_solver_test_prefix(solver, mid)) {
            low = mid;
        } else {
            high = mid - 1;
        }
    }

    sat_solver_destroy(solver);
    return low;
}
