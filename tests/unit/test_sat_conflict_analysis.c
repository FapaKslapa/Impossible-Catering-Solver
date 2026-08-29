#include "sat_conflict_analysis.h"
#include "sat_internal.h"
#include "formula.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    int a = 0, b = 1, c = 2, d = 3, e = 4;

    int pool[9];
    pool[0] = lit_neg(a); pool[1] = lit_neg(b); pool[2] = lit_pos(d);
    pool[3] = lit_neg(c); pool[4] = lit_neg(d); pool[5] = lit_pos(e);
    pool[6] = lit_neg(a); pool[7] = lit_neg(c); pool[8] = lit_neg(e);

    Clause clauses[3];
    clauses[0].offset = 0; clauses[0].size = 3; clauses[0].is_tautology = false;
    clauses[1].offset = 3; clauses[1].size = 3; clauses[1].is_tautology = false;
    clauses[2].offset = 6; clauses[2].size = 3; clauses[2].is_tautology = false;

    Formula formula;
    formula.variable_count = 5;
    formula.literal_pool = pool;
    formula.literal_pool_size = 9;
    formula.clauses = clauses;
    formula.clause_count = 3;

    SatSolver *solver = sat_solver_create(&formula);

    for (int i = 0; i < 5; i++) {
        solver->assignment[i] = VALUE_UNASSIGNED;
        solver->decision_level[i] = -1;
        solver->antecedent[i] = -1;
        solver->activity[i] = 0.0;
        solver->seen[i] = 0;
    }
    solver->seen_epoch = 0;
    solver->activity_increment = 1.0;
    solver->learned_clause_count = 0;
    solver->clause_activity_increment = 1.0;

    solver->current_decision_level = 1;
    solver->trail_size = 0;
    solver->trail[solver->trail_size++] = lit_pos(a);
    solver->assignment[a] = VALUE_TRUE;
    solver->decision_level[a] = 1;
    solver->antecedent[a] = -1;

    solver->current_decision_level = 2;
    solver->trail[solver->trail_size++] = lit_pos(b);
    solver->assignment[b] = VALUE_TRUE;
    solver->decision_level[b] = 2;
    solver->antecedent[b] = -1;

    solver->current_decision_level = 3;
    solver->trail[solver->trail_size++] = lit_pos(c);
    solver->assignment[c] = VALUE_TRUE;
    solver->decision_level[c] = 3;
    solver->antecedent[c] = -1;

    solver->trail[solver->trail_size++] = lit_pos(d);
    solver->assignment[d] = VALUE_TRUE;
    solver->decision_level[d] = 3;
    solver->antecedent[d] = 0;

    solver->trail[solver->trail_size++] = lit_pos(e);
    solver->assignment[e] = VALUE_TRUE;
    solver->decision_level[e] = 3;
    solver->antecedent[e] = 1;

    int learned_literals[8];
    int backjump_level;
    int learned_size = sat_analyze_conflict(solver, 2, learned_literals, &backjump_level);

    assert(learned_size == 3);
    assert(learned_literals[0] == lit_neg(c));
    bool has_neg_a = false;
    bool has_neg_b = false;
    for (int i = 1; i < learned_size; i++) {
        if (learned_literals[i] == lit_neg(a)) has_neg_a = true;
        if (learned_literals[i] == lit_neg(b)) has_neg_b = true;
    }
    assert(has_neg_a);
    assert(has_neg_b);
    assert(backjump_level == 2);

    sat_solver_destroy(solver);

    int f = 5;
    int pool2[11];
    pool2[0] = lit_neg(a); pool2[1] = lit_neg(b); pool2[2] = lit_pos(d);
    pool2[3] = lit_neg(c); pool2[4] = lit_neg(d); pool2[5] = lit_pos(e);
    pool2[6] = lit_neg(a); pool2[7] = lit_neg(c); pool2[8] = lit_neg(e);
    pool2[9] = lit_neg(d); pool2[10] = lit_pos(f);

    Clause clauses2[4];
    clauses2[0].offset = 0; clauses2[0].size = 3; clauses2[0].is_tautology = false;
    clauses2[1].offset = 3; clauses2[1].size = 3; clauses2[1].is_tautology = false;
    clauses2[2].offset = 6; clauses2[2].size = 3; clauses2[2].is_tautology = false;
    clauses2[3].offset = 9; clauses2[3].size = 2; clauses2[3].is_tautology = false;

    Formula formula2;
    formula2.variable_count = 6;
    formula2.literal_pool = pool2;
    formula2.literal_pool_size = 11;
    formula2.clauses = clauses2;
    formula2.clause_count = 4;

    SatSolver *solver2 = sat_solver_create(&formula2);

    for (int i = 0; i < 6; i++) {
        solver2->assignment[i] = VALUE_UNASSIGNED;
        solver2->decision_level[i] = -1;
        solver2->antecedent[i] = -1;
        solver2->activity[i] = 0.0;
        solver2->seen[i] = 0;
    }
    solver2->seen_epoch = 0;
    solver2->activity_increment = 1.0;
    solver2->learned_clause_count = 0;
    solver2->clause_activity_increment = 1.0;

    solver2->current_decision_level = 1;
    solver2->trail_size = 0;
    solver2->trail[solver2->trail_size++] = lit_pos(a);
    solver2->assignment[a] = VALUE_TRUE;
    solver2->decision_level[a] = 1;
    solver2->antecedent[a] = -1;

    solver2->current_decision_level = 2;
    solver2->trail[solver2->trail_size++] = lit_pos(b);
    solver2->assignment[b] = VALUE_TRUE;
    solver2->decision_level[b] = 2;
    solver2->antecedent[b] = -1;

    solver2->current_decision_level = 3;
    solver2->trail[solver2->trail_size++] = lit_pos(c);
    solver2->assignment[c] = VALUE_TRUE;
    solver2->decision_level[c] = 3;
    solver2->antecedent[c] = -1;

    solver2->trail[solver2->trail_size++] = lit_pos(d);
    solver2->assignment[d] = VALUE_TRUE;
    solver2->decision_level[d] = 3;
    solver2->antecedent[d] = 0;

    solver2->trail[solver2->trail_size++] = lit_pos(e);
    solver2->assignment[e] = VALUE_TRUE;
    solver2->decision_level[e] = 3;
    solver2->antecedent[e] = 1;

    solver2->trail[solver2->trail_size++] = lit_pos(f);
    solver2->assignment[f] = VALUE_TRUE;
    solver2->decision_level[f] = 3;
    solver2->antecedent[f] = 3;

    int learned_literals2[8];
    int backjump_level2;
    int learned_size2 = sat_analyze_conflict(solver2, 2, learned_literals2, &backjump_level2);

    assert(learned_size2 == 3);
    assert(learned_literals2[0] == lit_neg(c));
    bool has_neg_a2 = false;
    bool has_neg_b2 = false;
    for (int i = 1; i < learned_size2; i++) {
        if (learned_literals2[i] == lit_neg(a)) has_neg_a2 = true;
        if (learned_literals2[i] == lit_neg(b)) has_neg_b2 = true;
    }
    assert(has_neg_a2);
    assert(has_neg_b2);
    assert(backjump_level2 == 2);
    assert(solver2->assignment[f] == VALUE_TRUE);

    sat_solver_destroy(solver2);
    printf("test_sat_conflict_analysis: OK\n");
    return 0;
}
