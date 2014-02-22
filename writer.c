#include "writer.h"
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

void writer_init(writer *w, void *data, writefn fn)
{
	memset(w, 0, sizeof(writer));
	w->data = data;
	w->fn = fn;
}

void writer_reset(writer *w)
{
	memset(w, 0, sizeof(writer));
}

int writer_vprintf(writer *w, const char *fmt, va_list ap)
{
	va_list cpy;
	char *chunk;
	size_t len;

	va_copy(cpy, ap);
	len = vsnprintf(NULL, 0, fmt, cpy);
	va_end(cpy);

	chunk = malloc(len + 1);
	vsnprintf(chunk, len + 1, fmt, ap);
	writer_write(w, chunk, len);
	free(chunk);

	return len;
}

int writer_printf(writer *w, const char *fmt, ...)
{
	size_t len;
	va_list ap, cpy;
	char *chunk;

	va_start(ap, fmt);
	len = writer_vprintf(w, fmt, ap);
	va_end(ap);
	return len;
}

size_t writer_write(writer *w, const char *buf, size_t len)
{
	return w->fn(buf, len, 1, w->data) * len;
}
