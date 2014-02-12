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
	return x->writefn(ptr, len, 1, x->wdata);
}

int xprintf(printer *x, const char *fmt, ...)
{
	va_list ap;
	size_t len;
	char *buf;

	va_start(ap, fmt);
	len = vsnprintf(NULL, 0, fmt, ap);
	buf = malloc(len + 1);
	len = vsnprintf(buf, len, fmt, ap);
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
		len += xprintf(x, "%s: %s\r\n",
			field_get_name(f), field_get_value(f));
	}

	len += xprintf(x, "\r\n");

	return len;
}

size_t print_chunk(printer *x, char *src, size_t len)
{
	xwrite(x, src, len);
}
