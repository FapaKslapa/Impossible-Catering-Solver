#include "result_printer.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

static void read_all(FILE *file, char *buffer, size_t buffer_size) {
    rewind(file);
    size_t total = fread(buffer, 1, buffer_size - 1, file);
    buffer[total] = '\0';
}

int main(void) {
    FILE *file1 = tmpfile();
    print_result(file1, 2, 2);
    char output1[64];
    read_all(file1, output1, sizeof(output1));
    assert(strcmp(output1, "OK\n") == 0);
    fclose(file1);

    FILE *file2 = tmpfile();
    print_result(file2, 6, 4);
    char output2[64];
    read_all(file2, output2, sizeof(output2));
    assert(strcmp(output2, "KO\n-1\n-2\nOK\n") == 0);
    fclose(file2);

    FILE *file3 = tmpfile();
    print_result(file3, 0, 0);
    char output3[64];
    read_all(file3, output3, sizeof(output3));
    assert(strcmp(output3, "OK\n") == 0);
    fclose(file3);

    printf("test_result_printer: OK\n");
    return 0;
}
