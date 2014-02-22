#include "buffer.h"
#include <string.h>
#include <stdlib.h>

void buffer_init(buffer *b)
{
	memset(b, 0, sizeof(buffer));
}

void buffer_putc(buffer *b, int c)
{
	char ci;

	ci = (char)c;
	buffer_write(&ci, 1, 1, b);
}

void buffer_writer(buffer *b, writer *w)
{
	writer_init(w, (void *)b, (writefn)buffer_write);
}

void buffer_reset(buffer *b)
{
	memset(b, 0, sizeof(buffer));
}

size_t buffer_write(const char *chunk, size_t len, size_t num, buffer *b)
{
	len *= num;
	int changed = 0;

	while(b->cap <= b->len + len) {
		if(b->cap == 0)
			b->cap = 256;
		else
			b->cap *= 2;
		changed = 1;
	}
	if(changed)
		b->chunk = realloc(b->chunk, b->cap);

	memcpy(b->chunk + b->len, chunk, len);
	b->len += len;
	b->chunk[b->len] = '\0';

	return num;
}

unsigned char *buffer_get(buffer *b)
{
	return b->chunk;
}

unsigned char *buffer_get_n(buffer *b, size_t *rlen)
{
	*rlen = b->len;
	return b->chunk;
}

size_t buffer_len(buffer *b)
{
	return b->len;
}
