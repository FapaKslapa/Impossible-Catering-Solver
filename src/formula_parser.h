#ifndef FORMULA_PARSER_H
#define FORMULA_PARSER_H

#include <stdio.h>
#include <stdbool.h>
#include "formula.h"

bool formula_parse(FILE *file, Formula *out_formula);

#endif
