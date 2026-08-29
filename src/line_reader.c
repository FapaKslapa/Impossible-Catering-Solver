#include "line_reader.h"
#include <stdlib.h>

void line_buffer_init(LineBuffer *line) {
    line->data = NULL;
    line->capacity = 0;
    line->length = 0;
}

bool line_buffer_read(FILE *stream, LineBuffer *line) {
    line->length = getline(&line->data, &line->capacity, stream);
    if (line->length < 0) {
        return false;
    }
    while (line->length > 0 &&
           (line->data[line->length - 1] == '\n' ||
            line->data[line->length - 1] == '\r')) {
        line->length--;
    }
    line->data[line->length] = '\0';
    return true;
}

void line_buffer_destroy(LineBuffer *line) {
    free(line->data);
    line->data = NULL;
    line->capacity = 0;
    line->length = 0;
}
