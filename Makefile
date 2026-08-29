CC = clang
CFLAGS = -std=gnu11 -Wall -Werror -g
SRC_DIR = src
TEST_DIR = tests/unit

.PHONY: all clean test bernardo

bernardo: $(SRC_DIR)/main.c
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o bernardo $(SRC_DIR)/main.c

test:

clean:
	rm -f bernardo Lode Lode.c
	rm -f $(TEST_DIR)/*.o
