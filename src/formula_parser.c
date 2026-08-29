#include "formula_parser.h"
#include "line_reader.h"
#include "token_scanner.h"
#include "dish_table.h"
#include <stdlib.h>

#define FORMULA_INITIAL_CLAUSE_CAPACITY 16
#define FORMULA_INITIAL_POOL_CAPACITY 64
#define FORMULA_INITIAL_SCRATCH_CAPACITY 8

typedef struct {
    int *pool;
    int pool_size;
    int pool_capacity;
    Clause *clauses;
    int clause_count;
    int clause_capacity;
    int *scratch_raw;
    int *scratch_deduped;
    int scratch_capacity;
} FormulaBuilder;

static void formula_builder_init(FormulaBuilder *builder) {
    builder->pool_capacity = FORMULA_INITIAL_POOL_CAPACITY;
    builder->pool = malloc((size_t) builder->pool_capacity * sizeof(int));
    builder->pool_size = 0;
    builder->clause_capacity = FORMULA_INITIAL_CLAUSE_CAPACITY;
    builder->clauses = malloc((size_t) builder->clause_capacity * sizeof(Clause));
    builder->clause_count = 0;
    builder->scratch_capacity = FORMULA_INITIAL_SCRATCH_CAPACITY;
    builder->scratch_raw = malloc((size_t) builder->scratch_capacity * sizeof(int));
    builder->scratch_deduped = malloc((size_t) builder->scratch_capacity * sizeof(int));
}

static void formula_builder_free_scratch(FormulaBuilder *builder) {
    free(builder->scratch_raw);
    free(builder->scratch_deduped);
}

static void formula_builder_reserve_scratch(FormulaBuilder *builder, int needed) {
    if (needed > builder->scratch_capacity) {
        while (needed > builder->scratch_capacity) {
            builder->scratch_capacity *= 2;
        }
        builder->scratch_raw = realloc(builder->scratch_raw, (size_t) builder->scratch_capacity * sizeof(int));
        builder->scratch_deduped = realloc(builder->scratch_deduped, (size_t) builder->scratch_capacity * sizeof(int));
    }
}

static void formula_builder_reserve_pool(FormulaBuilder *builder, int extra) {
    if (builder->pool_size + extra > builder->pool_capacity) {
        while (builder->pool_size + extra > builder->pool_capacity) {
            builder->pool_capacity *= 2;
        }
        builder->pool = realloc(builder->pool, (size_t) builder->pool_capacity * sizeof(int));
    }
}

static void formula_builder_add_clause(FormulaBuilder *builder, int raw_count) {
    int deduped_count = 0;
    bool is_tautology = false;
    for (int i = 0; i < raw_count; i++) {
        int literal = builder->scratch_raw[i];
        bool already_present = false;
        for (int j = 0; j < deduped_count; j++) {
            if (builder->scratch_deduped[j] == literal) {
                already_present = true;
                break;
            }
            if (builder->scratch_deduped[j] == lit_negate(literal)) {
                is_tautology = true;
            }
        }
        if (!already_present) {
            builder->scratch_deduped[deduped_count] = literal;
            deduped_count++;
        }
    }

    if (builder->clause_count == builder->clause_capacity) {
        builder->clause_capacity *= 2;
        builder->clauses = realloc(builder->clauses, (size_t) builder->clause_capacity * sizeof(Clause));
    }

    Clause *clause = &builder->clauses[builder->clause_count];
    builder->clause_count++;
    clause->is_tautology = is_tautology;
    if (is_tautology) {
        clause->offset = 0;
        clause->size = 0;
        return;
    }

    formula_builder_reserve_pool(builder, deduped_count);
    clause->offset = builder->pool_size;
    clause->size = deduped_count;
    for (int i = 0; i < deduped_count; i++) {
        builder->pool[builder->pool_size] = builder->scratch_deduped[i];
        builder->pool_size++;
    }
}

static bool parse_request_line(char *line, DishTable *dishes, FormulaBuilder *builder) {
    TokenList tokens;
    tokenize_line(line, &tokens);
    if (tokens.count == 0) {
        token_list_destroy(&tokens);
        return true;
    }
    formula_builder_reserve_scratch(builder, (int) tokens.count);
    bool ok = true;
    for (size_t i = 0; i < tokens.count; i++) {
        char *token = tokens.tokens[i];
        bool negative = (token[0] == '-');
        const char *dish_name = negative ? token + 1 : token;
        int variable_index;
        if (!dish_table_try_get(dishes, dish_name, &variable_index)) {
            ok = false;
            break;
        }
        builder->scratch_raw[i] = negative ? lit_neg(variable_index) : lit_pos(variable_index);
    }
    if (ok) {
        formula_builder_add_clause(builder, (int) tokens.count);
    }
    token_list_destroy(&tokens);
    return ok;
}

bool formula_parse(FILE *file, Formula *out_formula) {
    LineBuffer line;
    line_buffer_init(&line);

    if (!line_buffer_read(file, &line)) {
        line_buffer_destroy(&line);
        return false;
    }

    DishTable *dishes = dish_table_create();
    TokenList dish_tokens;
    tokenize_line(line.data, &dish_tokens);
    for (size_t i = 0; i < dish_tokens.count; i++) {
        dish_table_get_or_add(dishes, dish_tokens.tokens[i]);
    }
    token_list_destroy(&dish_tokens);

    FormulaBuilder builder;
    formula_builder_init(&builder);

    bool success = true;
    while (line_buffer_read(file, &line)) {
        if (line.length == 0) {
            continue;
        }
        if (!parse_request_line(line.data, dishes, &builder)) {
            success = false;
            break;
        }
    }

    line_buffer_destroy(&line);
    formula_builder_free_scratch(&builder);

    int variable_count = dish_table_count(dishes);
    dish_table_destroy(dishes);

    if (!success) {
        free(builder.pool);
        free(builder.clauses);
        return false;
    }

    int final_pool_size = builder.pool_size > 0 ? builder.pool_size : 1;
    int final_clause_count = builder.clause_count > 0 ? builder.clause_count : 1;
    builder.pool = realloc(builder.pool, (size_t) final_pool_size * sizeof(int));
    builder.clauses = realloc(builder.clauses, (size_t) final_clause_count * sizeof(Clause));

    out_formula->literal_pool = builder.pool;
    out_formula->literal_pool_size = builder.pool_size;
    out_formula->clauses = builder.clauses;
    out_formula->clause_count = builder.clause_count;
    out_formula->variable_count = variable_count;
    return true;
}
