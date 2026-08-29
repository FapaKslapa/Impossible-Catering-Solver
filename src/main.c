#include <stdio.h>
#include <stdbool.h>
#include "formula.h"
#include "formula_parser.h"
#include "threshold_search.h"
#include "result_printer.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <input-file>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        fprintf(stderr, "cannot open file: %s\n", argv[1]);
        return 1;
    }

    Formula formula;
    bool parsed = formula_parse(file, &formula);
    fclose(file);

    if (!parsed) {
        fprintf(stderr, "invalid input format\n");
        return 1;
    }

    int max_prefix = find_max_satisfiable_prefix(&formula);
    print_result(stdout, formula.clause_count, max_prefix);

    formula_destroy(&formula);
    return 0;
}
