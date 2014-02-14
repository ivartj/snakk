#include "msg.h"
#include "printer.h"
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

void printer_init(printer *x, printer_writefn writefn, void *wdata)
{
	memset(x, 0, sizeof(printer));
	x->writefn = writefn;
	x->wdata = wdata;
}

size_t xwrite(printer *x, const char *ptr, size_t len)
{
	return x->writefn(ptr, 1, len, x->wdata);
}

int xprintf(printer *x, const char *fmt, ...)
{
	va_list ap, cpy;
	size_t len;
	char *buf;

	va_start(ap, fmt);

	va_copy(cpy, ap);
	len = vsnprintf(NULL, 0, fmt, cpy);
	va_end(cpy);

	buf = malloc(len + 1);

	va_copy(cpy, ap);
	len = vsnprintf(buf, len + 1, fmt, ap);
	va_end(cpy);

	buf[len] = '\0';
	len = xwrite(x, buf, len);
	free(buf);
	va_end(ap);
	return len;
}

size_t print_msg(printer *x, msg *m)
{
	header *hd;
	size_t len = 0;
	char *body;
	size_t bodylen;

	hd = msg_get_header(m);
	len += print_header(x, hd);
	len += xprintf(x, "\r\n");
	body = msg_get_body_n(m, &bodylen);
	len += xwrite(x, body, bodylen);
	return len;
}

size_t print_header(printer *x, header *hd)
{
	field *f;
	int i, fc;
	size_t len = 0;

	fc = header_get_field_num(hd);
	for(i = 0; i < fc; i++) {
		f = header_get_field_i(hd, i);
		len += print_field(x, f);
	}

	return len;
}

size_t print_field(printer *x, field *f)
{
	xprintf(x, "%s: %s\r\n", field_get_name(f), field_get_value(f));
}

size_t print_field_verbatim(printer *x, field *f)
{
	char *src;
	size_t len;

	src = field_get_src_n(f, &len);
	print_chunk(x, src, len);
}

size_t print_chunk(printer *x, char *src, size_t len)
{
	xwrite(x, src, len);
}
