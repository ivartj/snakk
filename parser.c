#include "parser.h"
#include "buffer.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

void parser_init(parser *p, parser_readfn readfn, void *rdata)
{
	memset(p, 0, sizeof(parser));
	p->readfn = readfn;
	p->rdata = rdata;
}

void parser_reset(parser *p)
{
	if(p->src)
		free(p->src);
	memset(p, 0, sizeof(parser));
}

size_t xread(parser *x, void *data, size_t len)
{
	size_t dec, off;

	off = 0;

	if(x->srcoff < x->srclen) {
		dec = len < x->srclen - x->srcoff ? len : x->srclen - x->srcoff;
		memcpy(data, x->src + x->srcoff, dec);
		x->srcoff += off += dec;
	}

	if(off == len)
		return off;

	dec = x->readfn(data + off, 1, len - off, x->rdata);
	if(dec == 0)
		return off;

	xadd(x, data + off, dec);
	off += dec;

	return off;
}

int xgetc(parser *x)
{
	unsigned char c;
	size_t len;

	len = xread(x, &c, 1);
	if(len == 0)
		return EOF;

	return c;
}

size_t xget(parser *x)
{
	return x->srcoff;
}

void xset(parser *x, size_t off)
{
	x->srcoff = off;
}

void xdec(parser *x)
{
	xset(x, xget(x) - 1);
}

void xadd(parser *x, void *data, size_t len)
{
	int ch;

	ch = 0;
	while(x->srclen + len >= x->srccap) {
		if(x->srccap == 0)
			x->srccap = 256;
		else
			x->srccap *= 2;
		ch = 1;
	}
	if(ch)
		x->src = realloc(x->src, x->srccap);

	memcpy(x->src + x->srclen, data, len);
	x->srcoff = x->srclen += len;
}

char *xcpy(parser *x, size_t start, size_t stop)
{
	size_t len;
	void *cpy;

	len = stop - start;
	cpy = calloc(1, len + 1);
	memcpy(cpy, x->src + start, len);
	return cpy;
}

char *parse_full_n(parser *x, size_t *rlen)
{
	size_t start;
	char buf[256];

	start = xget(x);

	while(xread(x, buf, sizeof(buf)) == sizeof(buf)); 

	*rlen = xget(x) - start;

	return xcpy(x, start, xget(x));
}

int parse_linebreak(parser *x)
{
	size_t start;
	int c;
	enum {
		st_sp,
		st_r,
	} state;

	start = xget(x);

	while((c = xgetc(x)) != EOF)
	switch(state) {
	case st_sp:
		switch(c) {
		case ' ':
		case '\t':
			break;
		case '\n':
			goto ret;
		case '\r':
			state = st_r;
			break;
		default:
			goto abort;
		}
		break;
	case st_r:
		switch(c) {
		case '\n':
			goto ret;
		default:
			goto abort;
		}
		break;
	}

abort:
	xset(x, start);
	return 0;
ret:
	return xget(x) - start;
}

void xerr(parser *x, const char *fmt, ...)
{
	va_list ap;
	char cpy[PARSER_ERRLEN];

	va_start(ap, fmt);
	vsnprintf(cpy, PARSER_ERRLEN, fmt, ap);
	memcpy(x->errmsg, cpy, PARSER_ERRLEN);
	va_end(ap);
}

int parse_linespace(parser *x)
{
	size_t start;
	int c;

	start = xget(x);

	while((c = xgetc(x)) != EOF)
	switch(c) {
	case '\t':
	case ' ':
		break;
	default:
		xdec(x);
		goto ret;
	}
ret:
	return xget(x) - start;
}

int parse_text(parser *x, const char *text)
{
	size_t start;
	int c;

	start = xget(x);

	while(*text != '\0') {
		c = xgetc(x);
		if(*text == c)
			text++;
		else {
			if(c == EOF)
				xerr(x, "Unexpected end of line when expecting '%s'", text);
			else
				xerr(x, "Unexpected character '%c' when expecting '%c'", c, *text);
			goto abort;
		}
	}

	return xget(x) - start;
abort:
	xset(x, start);
	return 0;
}


/* ############# *
 * ### FIELD ### *
 * ############# */

#define A_NOTCOLON(c) ((c >= 0x21 && c <= 0x39) || (c >= 0x3B && c <= 0x7E))
#define S_CHAR(c) (c >= 0x21 && c <= 0xFF)
#define LWS(c) (c == ' ' || c == '\t')

static int parse_field_name(parser *x, char **rname, size_t *rnamelen);
static int parse_field_colon(parser *x);
static int parse_field_value(parser *x, char **rvalue, size_t *rlen);

field *parse_field(parser *x)
{
	size_t start;
	field *fld;
	buffer b;
	char *value = NULL;
	char *name = NULL;
	size_t valuelen = 0;
	size_t namelen = 0;

	start = xget(x);

	if(!parse_field_name(x, &name, &namelen)) {
		xerr(x, "Failed to parse field name");
		goto abort;
	}

	if(!parse_text(x, ":")) {
		xerr(x, "Unexpected character in field name.");
		goto abort;
	}

	if(!parse_field_value(x, &value, &valuelen)) {
		xerr(x, "Failed to parse field value");
		goto abort;
	} else
		goto ret;

abort:
	if(name != NULL)
		free(name);
	if(value != NULL)
		free(value);
	xset(x, start);
	return NULL;
ret:
	fld = field_create();
	field_set_name_n(fld, name, namelen);
	field_set_value_n(fld, value, valuelen);
	field_set_src_on(fld, x->src, start, xget(x));

	return fld;
}

int parse_field_name(parser *x, char **rname, size_t *rnamelen)
{
	size_t start;
	int c;
	buffer b;
	int capital = 1;
	enum {
		st_init,
		st_name,
	} state;

	start = xget(x);
	state = st_init;
	buffer_init(&b);

	while((c = xgetc(x)) != EOF)
	switch(state) {
	case st_init:
		if(A_NOTCOLON(c)) {
			state = st_name;
			if(isalpha(c)) {
				c = toupper(c);
				capital = 0;
			}
			buffer_putc(&b, c);
			break;
		}
		xerr(x, "Unexpected character '%c' at start of field name", c);
		goto abort;
	case st_name:
		if(A_NOTCOLON(c)) {
			if(isalpha(c)) {
				c = capital ? toupper(c) : tolower(c);
				capital = 0;
			} else
				capital = 1;
			buffer_putc(&b, c);
			break;
		}

		xdec(x);
		goto ret;
	}
abort:
	buffer_reset(&b);
	xset(x, start);
	return 0;
ret:
	*rname = buffer_get_n(&b, rnamelen);
	return 1;
}

int parse_field_value(parser *x, char **rvalue, size_t *rlen)
{
	size_t start, line;
	int c;
	buffer b;
	enum {
		st_start,
		st_start_r,
		st_r,
		st_line,
		st_line_r,
		st_space,
		st_value,
	} state;

	start = xget(x);
	state = st_start;
	buffer_init(&b);

	while((c = xgetc(x)) != EOF)
	switch(state) {
	case st_start:

		if(LWS(c))
			break;

		if(S_CHAR(c)) {
			buffer_putc(&b, c);
			state = st_value;
			break;
		}

		if(c == '\r') {
			state = st_r;
			break;
		}

		if(c == '\n') {
			state = st_line;
			line = xget(x);
			break;
		}

		xerr(x, "Unexpected character '%c' at start of field value", c);
		goto abort;
	case st_r:

		if(c == '\n') {
			state = st_line;
			line = xget(x);
			break;
		}
		
		xerr(x, "Expected newline instead of '%c' after carriage return", c);
		goto abort;
	case st_line:

		if(LWS(c)) {
			state = st_space;
			break;
		}

		if(c == '\r') {
			state = st_line_r;
			break;
		}

		xset(x, line);
		goto ret;

	case st_space:

		if(LWS(c))
			break;

		if(S_CHAR(c)) {
			if(buffer_len(&b) != 0)
				buffer_putc(&b, ' ');
			buffer_putc(&b, c);
			state = st_value;
			break;
		}

		if(c == '\r') {
			state = st_line_r;
			break;
		}

		if(c == '\n') {
			xset(x, line);
			goto ret;
		}

		xerr(x, "Unexpected character '%c' after line break", c);
		goto abort;
	case st_line_r:

		if(c == '\n') {
			xset(x, line);
			goto ret;
		}

		xerr(x, "Expected newline instead of '%c' after carriage return", c);
		goto abort;
	case st_value:

		if(S_CHAR(c) || LWS(c)) {
			buffer_putc(&b, c);
			break;
		}

		if(c == '\r') {
			state = st_r;
			break;
		}

		if(c == '\n') {
			state = st_line;
			line = xget(x);
			break;
		}

		xerr(x, "Unexpected character '%c' in field value", c);
		goto abort;
	}
abort:
	buffer_reset(&b);
	xset(x, start);
	return 0;
ret:
	*rvalue = buffer_get_n(&b, rlen);
	return 1;
}


/* ############## *
 * ### HEADER ### *
 * ############## */

header *parse_header(parser *x)
{
	size_t start;
	header *hd;
	field *f;

	hd = header_create();
	start = xget(x);

	while((f = parse_field(x)) != NULL)
		header_add_field(hd, f);
	goto ret;

abort:
	header_destroy(hd);
	xset(x, start);
	return NULL;
ret:
	header_set_src_on(hd, x->src, start, xget(x));
	return hd;
}

/* ############### *
 * ### MESSAGE ### *
 * ############### */

static header *parse_msg_sig(parser *x);
static header *parse_msg_header(parser *x);

msg *parse_msg(parser *x)
{
	size_t start;
	msg *m;
	header *hd;
	char *body;
	size_t bodylen;

	m = msg_create();
	start = xget(x);

	if((hd = parse_header(x)) == NULL) {
		xerr(x, "Failed to parse header: %s", x->errmsg);
		goto abort;
	}

	msg_set_header(m, hd);

	if(!parse_linebreak(x)) {
		xerr(x, "No expected line break after header");
		goto abort;
	}

	if((body = parse_full_n(x, &bodylen)) == NULL) {
		xerr(x, "Failed to parse message body.");
		goto abort;
	}

	msg_set_body_n(m, body, bodylen);

	goto ret;

abort:
	msg_destroy(m);
	xset(x, start);
	return NULL;
ret:
	msg_set_src_on(m, x->src, start, xget(x));
	return m;
}
