#!/bin/sh
set -e

mkdir -p out
OUT=out/Lode.c
: > "$OUT"

HEADERS="src/line_reader.h src/token_scanner.h src/dish_table.h src/formula.h src/formula_parser.h src/sat_solver.h src/sat_restart.h src/sat_clause_db.h src/sat_conflict_analysis.h src/sat_internal.h src/threshold_search.h src/result_printer.h"
SOURCES="src/line_reader.c src/token_scanner.c src/dish_table.c src/formula.c src/formula_parser.c src/sat_propagate.c src/sat_restart.c src/sat_clause_db.c src/sat_conflict_analysis.c src/sat_solver.c src/threshold_search.c src/result_printer.c src/main.c"

for f in $HEADERS; do
    grep -v '^#include "' "$f" >> "$OUT"
    printf '\n' >> "$OUT"
done

for f in $SOURCES; do
    grep -v '^#include "' "$f" >> "$OUT"
    printf '\n' >> "$OUT"
done

echo "Wrote $OUT"
