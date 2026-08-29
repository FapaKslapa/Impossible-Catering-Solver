CC = clang
CFLAGS = -std=gnu11 -Wall -Werror -g
SRC_DIR = src
TEST_DIR = tests/unit

.PHONY: all clean test bernardo

BERNARDO_SOURCES = $(SRC_DIR)/line_reader.c $(SRC_DIR)/token_scanner.c $(SRC_DIR)/dish_table.c \
                    $(SRC_DIR)/formula.c $(SRC_DIR)/formula_parser.c $(SRC_DIR)/sat_propagate.c \
                    $(SRC_DIR)/sat_solver.c $(SRC_DIR)/threshold_search.c $(SRC_DIR)/result_printer.c

bernardo: $(SRC_DIR)/main.c $(BERNARDO_SOURCES)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o bernardo $(SRC_DIR)/main.c $(BERNARDO_SOURCES)

test_line_reader: $(TEST_DIR)/test_line_reader.c $(SRC_DIR)/line_reader.c
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $^

test_token_scanner: $(TEST_DIR)/test_token_scanner.c $(SRC_DIR)/token_scanner.c
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $^

test_dish_table: $(TEST_DIR)/test_dish_table.c $(SRC_DIR)/dish_table.c
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $^

test_formula_parser: $(TEST_DIR)/test_formula_parser.c $(SRC_DIR)/formula_parser.c $(SRC_DIR)/formula.c $(SRC_DIR)/line_reader.c $(SRC_DIR)/token_scanner.c $(SRC_DIR)/dish_table.c
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $^

test_sat_propagate: $(TEST_DIR)/test_sat_propagate.c $(SRC_DIR)/sat_propagate.c $(SRC_DIR)/sat_solver.c $(SRC_DIR)/formula.c
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $^

test_sat_solver: $(TEST_DIR)/test_sat_solver.c $(SRC_DIR)/sat_solver.c $(SRC_DIR)/sat_propagate.c $(SRC_DIR)/formula.c
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $^

test_threshold_search: $(TEST_DIR)/test_threshold_search.c $(SRC_DIR)/threshold_search.c $(SRC_DIR)/sat_solver.c $(SRC_DIR)/sat_propagate.c $(SRC_DIR)/formula.c
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $^

test_result_printer: $(TEST_DIR)/test_result_printer.c $(SRC_DIR)/result_printer.c
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $^

test_sat_restart: $(TEST_DIR)/test_sat_restart.c $(SRC_DIR)/sat_restart.c
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $^

test: test_line_reader test_token_scanner test_dish_table test_formula_parser test_sat_propagate test_sat_solver test_threshold_search test_result_printer test_sat_restart
	./test_line_reader
	./test_token_scanner
	./test_dish_table
	./test_formula_parser
	./test_sat_propagate
	./test_sat_solver
	./test_threshold_search
	./test_result_printer
	./test_sat_restart

clean:
	rm -f bernardo Lode Lode.c
	rm -f $(TEST_DIR)/*.o
