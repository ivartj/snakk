#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "msg.h"

#define PARSER_ERRLEN 256

typedef struct _parser parser;

typedef size_t (*parser_readfn)(void *ptr, size_t size, size_t nitems, void *rdata);
typedef size_t (*parser_writefn)(const void *ptr, size_t size, size_t nitems, void *rdata);

struct _parser {
	char *src;
	size_t srclen, srcoff, srccap;
	parser_readfn readfn;
	void *rdata;
	char errmsg[PARSER_ERRLEN];
};

void parser_init(parser *p, parser_readfn readfn, void *rdata);
void parser_reset(parser *p);

size_t xread(parser *x, void *data, size_t len);
int xgetc(parser *x);
size_t xget(parser *x);
void xset(parser *x, size_t off);
void xadd(parser *x, void *data, size_t len);
char *xcpy(parser *x, size_t start, size_t stop);

static char *parse_full_n(parser *x, size_t *rlen);
static int parse_linespace(parser *x);
static int parse_text(parser *x, const char *text);


int parse_linebreak(parser *x);

/* ############# *
 * ### FIELD ### *
 * ############# */

field *parse_field(parser *x);

/* ############## *
 * ### HEADER ### *
 * ############## */

header *parse_header(parser *x);

/* ############### *
 * ### MESSAGE ### *
 * ############### */

msg *parse_msg(parser *x);

#endif
