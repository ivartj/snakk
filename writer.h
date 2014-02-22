#ifndef WRITER_H
#define WRITER_H

#include <stdio.h>

typedef struct _writer writer;
typedef size_t (*writefn)(const char *chunk, size_t len, size_t num, void *wdata);

struct _writer {
	writefn fn;
	void *data;
};

void writer_init(writer *w, void *data, writefn fn);
void writer_reset(writer *w);

int writer_vprintf(writer *w, const char *fmt, va_list ap);
int writer_printf(writer *w, const char *fmt, ...);
size_t writer_write(writer *w, const char *buf, size_t len);

#endif
