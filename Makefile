CC = clang
CFLAGS = -std=gnu11 -Wall -Werror -g
SRC_DIR = src
TEST_DIR = tests/unit
OUT_DIR = out

.PHONY: all clean test bernardo

BERNARDO_SOURCES = $(SRC_DIR)/line_reader.c $(SRC_DIR)/token_scanner.c $(SRC_DIR)/dish_table.c \
                    $(SRC_DIR)/formula.c $(SRC_DIR)/formula_parser.c $(SRC_DIR)/sat_propagate.c \
                    $(SRC_DIR)/sat_restart.c $(SRC_DIR)/sat_clause_db.c $(SRC_DIR)/sat_conflict_analysis.c \
                    $(SRC_DIR)/sat_solver.c $(SRC_DIR)/threshold_search.c $(SRC_DIR)/result_printer.c

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

bernardo: $(SRC_DIR)/main.c $(BERNARDO_SOURCES) | $(OUT_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $(OUT_DIR)/$@ $(SRC_DIR)/main.c $(BERNARDO_SOURCES)

test_line_reader: $(TEST_DIR)/test_line_reader.c $(SRC_DIR)/line_reader.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $(OUT_DIR)/$@ $^

test_token_scanner: $(TEST_DIR)/test_token_scanner.c $(SRC_DIR)/token_scanner.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $(OUT_DIR)/$@ $^

test_dish_table: $(TEST_DIR)/test_dish_table.c $(SRC_DIR)/dish_table.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $(OUT_DIR)/$@ $^

test_formula_parser: $(TEST_DIR)/test_formula_parser.c $(SRC_DIR)/formula_parser.c $(SRC_DIR)/formula.c $(SRC_DIR)/line_reader.c $(SRC_DIR)/token_scanner.c $(SRC_DIR)/dish_table.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $(OUT_DIR)/$@ $^

test_sat_propagate: $(TEST_DIR)/test_sat_propagate.c $(SRC_DIR)/sat_propagate.c $(SRC_DIR)/sat_solver.c $(SRC_DIR)/sat_restart.c $(SRC_DIR)/sat_clause_db.c $(SRC_DIR)/sat_conflict_analysis.c $(SRC_DIR)/formula.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $(OUT_DIR)/$@ $^

test_sat_solver: $(TEST_DIR)/test_sat_solver.c $(SRC_DIR)/sat_solver.c $(SRC_DIR)/sat_propagate.c $(SRC_DIR)/sat_restart.c $(SRC_DIR)/sat_clause_db.c $(SRC_DIR)/sat_conflict_analysis.c $(SRC_DIR)/formula.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $(OUT_DIR)/$@ $^

test_sat_clause_db: $(TEST_DIR)/test_sat_clause_db.c $(SRC_DIR)/sat_clause_db.c $(SRC_DIR)/sat_propagate.c $(SRC_DIR)/sat_solver.c $(SRC_DIR)/sat_restart.c $(SRC_DIR)/sat_conflict_analysis.c $(SRC_DIR)/formula.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $(OUT_DIR)/$@ $^

test_threshold_search: $(TEST_DIR)/test_threshold_search.c $(SRC_DIR)/threshold_search.c $(SRC_DIR)/sat_solver.c $(SRC_DIR)/sat_propagate.c $(SRC_DIR)/sat_restart.c $(SRC_DIR)/sat_clause_db.c $(SRC_DIR)/sat_conflict_analysis.c $(SRC_DIR)/formula.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $(OUT_DIR)/$@ $^

test_result_printer: $(TEST_DIR)/test_result_printer.c $(SRC_DIR)/result_printer.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $(OUT_DIR)/$@ $^

test_sat_restart: $(TEST_DIR)/test_sat_restart.c $(SRC_DIR)/sat_restart.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $(OUT_DIR)/$@ $^

test_sat_conflict_analysis: $(TEST_DIR)/test_sat_conflict_analysis.c $(SRC_DIR)/sat_conflict_analysis.c $(SRC_DIR)/sat_propagate.c $(SRC_DIR)/sat_solver.c $(SRC_DIR)/sat_restart.c $(SRC_DIR)/sat_clause_db.c $(SRC_DIR)/formula.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $(OUT_DIR)/$@ $^

test_sat_undo_to_level: $(TEST_DIR)/test_sat_undo_to_level.c $(SRC_DIR)/sat_solver.c $(SRC_DIR)/sat_propagate.c $(SRC_DIR)/sat_restart.c $(SRC_DIR)/sat_clause_db.c $(SRC_DIR)/sat_conflict_analysis.c $(SRC_DIR)/formula.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $(OUT_DIR)/$@ $^

test: test_line_reader test_token_scanner test_dish_table test_formula_parser test_sat_propagate test_sat_solver test_threshold_search test_result_printer test_sat_restart test_sat_clause_db test_sat_conflict_analysis test_sat_undo_to_level
	$(OUT_DIR)/test_line_reader
	$(OUT_DIR)/test_token_scanner
	$(OUT_DIR)/test_dish_table
	$(OUT_DIR)/test_formula_parser
	$(OUT_DIR)/test_sat_propagate
	$(OUT_DIR)/test_sat_solver
	$(OUT_DIR)/test_threshold_search
	$(OUT_DIR)/test_result_printer
	$(OUT_DIR)/test_sat_restart
	$(OUT_DIR)/test_sat_clause_db
	$(OUT_DIR)/test_sat_conflict_analysis
	$(OUT_DIR)/test_sat_undo_to_level

clean:
	rm -rf $(OUT_DIR)
