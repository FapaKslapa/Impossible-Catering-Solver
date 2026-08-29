CC = clang
CFLAGS = -std=gnu11 -Wall -Werror -g
SRC_DIR = src
TEST_DIR = tests/unit

.PHONY: all clean test bernardo

bernardo: $(SRC_DIR)/main.c
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o bernardo $(SRC_DIR)/main.c

test_line_reader: $(TEST_DIR)/test_line_reader.c $(SRC_DIR)/line_reader.c
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $^

test_token_scanner: $(TEST_DIR)/test_token_scanner.c $(SRC_DIR)/token_scanner.c
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $^

test_dish_table: $(TEST_DIR)/test_dish_table.c $(SRC_DIR)/dish_table.c
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $^

test_formula_parser: $(TEST_DIR)/test_formula_parser.c $(SRC_DIR)/formula_parser.c $(SRC_DIR)/formula.c $(SRC_DIR)/line_reader.c $(SRC_DIR)/token_scanner.c $(SRC_DIR)/dish_table.c
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $^

test: test_line_reader test_token_scanner test_dish_table test_formula_parser
	./test_line_reader
	./test_token_scanner
	./test_dish_table
	./test_formula_parser

clean:
	rm -f bernardo Lode Lode.c
	rm -f $(TEST_DIR)/*.o
