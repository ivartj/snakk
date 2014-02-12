#ifndef PRINTER_H
#define PRINTER_H

#include "msg.h"
#include <stdio.h>

typedef struct _printer printer;
typedef size_t (*printer_writefn)(const void *ptr, size_t size, size_t nitems, void *rdata);

struct _printer {
	void *wdata;
	printer_writefn writefn;
};

void printer_init(printer *x, printer_writefn writefn, void *wdata);

size_t print_msg(printer *x, msg *m);
size_t print_header(printer *x, header *hd);
size_t print_chunk(printer *x, char *src, size_t len);

#endif
