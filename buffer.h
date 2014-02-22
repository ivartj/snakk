#ifndef BUFFER_H
#define BUFFER_H

#include <stdio.h>
#include "writer.h"

typedef struct _buffer buffer;

struct _buffer {
	unsigned char *chunk;
	size_t len, cap;
};

void buffer_init(buffer *b);
void buffer_writer(buffer *b, writer *w);
void buffer_reset(buffer *b);

size_t buffer_write(const char *chunk, size_t len, size_t num, buffer *b);

unsigned char *buffer_get(buffer *b);
unsigned char *buffer_get_n(buffer *b, size_t *rlen);

#endif
