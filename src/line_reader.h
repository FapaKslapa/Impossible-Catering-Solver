#ifndef LINE_READER_H
#define LINE_READER_H

#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>

typedef struct {
    char *data;
    size_t capacity;
    ssize_t length;
} LineBuffer;

void line_buffer_init(LineBuffer *line);
bool line_buffer_read(FILE *stream, LineBuffer *line);
void line_buffer_destroy(LineBuffer *line);

#endif
