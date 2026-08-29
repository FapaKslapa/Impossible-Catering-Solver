#include "formula.h"
#include <stdlib.h>

void formula_destroy(Formula *formula) {
    free(formula->literal_pool);
    free(formula->clauses);
    formula->literal_pool = NULL;
    formula->clauses = NULL;
}
