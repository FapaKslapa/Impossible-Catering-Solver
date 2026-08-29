#include "result_printer.h"

void print_result(FILE *stream, int total_employees, int max_satisfiable_prefix) {
    if (max_satisfiable_prefix == total_employees) {
        fprintf(stream, "OK\n");
        return;
    }
    fprintf(stream, "KO\n");
    int removed = total_employees - max_satisfiable_prefix;
    for (int i = 1; i <= removed; i++) {
        fprintf(stream, "-%d\n", i);
    }
    fprintf(stream, "OK\n");
}
