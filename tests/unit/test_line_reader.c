#include "line_reader.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    FILE *file = tmpfile();
    fputs("first line\nsecond\n\nlast no newline", file);
    rewind(file);

    LineBuffer line;
    line_buffer_init(&line);

    assert(line_buffer_read(file, &line));
    assert(strcmp(line.data, "first line") == 0);

    assert(line_buffer_read(file, &line));
    assert(strcmp(line.data, "second") == 0);

    assert(line_buffer_read(file, &line));
    assert(strcmp(line.data, "") == 0);

    assert(line_buffer_read(file, &line));
    assert(strcmp(line.data, "last no newline") == 0);

    assert(!line_buffer_read(file, &line));

    line_buffer_destroy(&line);
    fclose(file);

    printf("test_line_reader: OK\n");
    return 0;
}
