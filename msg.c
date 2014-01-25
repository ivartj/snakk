#include "msg.h"
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>

typedef struct _msg_header msg_header;

struct _msg_header {
	char *name;
	size_t namelen;
	char *content;
	size_t contentlen;
};

struct _snakk_msg {
	int compiled;
	char *tmp;
	unsigned char *src;
	size_t srclen;
	size_t srcoff;
	size_t srccap;
	snakk_msg_readfn readfn;
	void *rdata;
	char *sig;
	size_t siglen;
	size_t signed_off;
	msg_header *hs;
	size_t hn;
	char *body;
	size_t bodylen;
};

static void h_append(snakk_msg *m, size_t name_start, size_t name_end, size_t content_start, size_t content_end);
static int parse(snakk_msg *m);
static int parseheader(snakk_msg *m);
static void parsebody(snakk_msg *m);
static size_t xread(snakk_msg *m, void *data, size_t len);
static void xadd(snakk_msg *m, void *data, size_t len);
static void xset(snakk_msg *m, size_t off);
static size_t xget(snakk_msg *m);
static void set_error(const char *fmt, ...);

static char error_string[512] = { 0 };

snakk_msg *snakk_msg_create(void)
{
	snakk_msg *msg;

	msg = calloc(1, sizeof(snakk_msg));

	return msg;
}

void xadd(snakk_msg *m, void *data, size_t len)
{
	int ch;

	ch = 0;
	while(m->srclen + len >= m->srccap) {
		if(m->srccap == 0)
			m->srccap = 256;
		else
			m->srccap *= 2;
		ch = 1;
	}
	if(ch)
		m->src = realloc(m->src, m->srccap);

	memcpy(m->src + m->srclen, data, len);
	m->srcoff = m->srclen += len;
}

void xset(snakk_msg *m, size_t off)
{
	m->srcoff = off;
}

size_t xget(snakk_msg *m)
{
	return m->srcoff;
}

size_t xread(snakk_msg *m, void *data, size_t len)
{
	size_t dec, off;

	off = 0;

	if(m->srcoff < m->srclen) {
		dec = len < m->srclen - m->srcoff ? len : m->srclen - m->srcoff;
		memcpy(data, m->src + m->srcoff, dec);
		m->srcoff += off += dec;
	}

	if(off == len)
		return off;

	dec = m->readfn(data + off, 1, len - off, m->rdata);
	if(dec == 0)
		return off;

	xadd(m, data + off, dec);
	off += dec;

	return off;
}

int xgetc(snakk_msg *m)
{
	unsigned char c;
	size_t len;

	len = xread(m, &c, 1);
	if(len == 0)
		return EOF;

	return c;
}

int parsesignature(snakk_msg *m)
{
	int c;
	size_t off, len;
	enum {
		st_sig,
		st_sig_r,
	} state = st_sig;

	len = 0;
	off = xget(m);

	while((c = xgetc(m)) != EOF)
	switch(state) {
	case st_sig:
		len++;
		switch(c) {
		case '\r':
			state = st_sig_r;
			break;
		case '\n':
			goto ret;
		default:
			if(!(c >= 0x20 && c <= 0x7e)) {
				set_error("Non-ASCII character '%c' in signature", c);
				goto abort;
			}
			break;
		}
		break;
	case st_sig_r:
		if(c == '\n')
			goto ret;
		goto abort;
	}
abort:
	return 0;

ret:
	m->sig = calloc(1, len + 1);
	memcpy(m->sig, m->src + off, len);
	m->siglen = len;
	m->signed_off = xget(m);

	return 1;
}

int parseheader(snakk_msg *m)
{
	size_t start, off;
	size_t name_start, name_stop;
	size_t content_start, content_stop;
	int c;
	enum {
		st_init,
		st_init_r,
		st_name,
		st_colon,
		st_colon_r,
		st_colon_n,
		st_content,
		st_content_r,
		st_content_n,
	} state = st_init;

	start = off = xget(m);

	while((c = xgetc(m)) != EOF)
	switch(state) {
	case st_init:

		// A-NOTCOLON
		if((c >= 0x21 && c <= 0x39) || (c >= 0x3B && c <= 0x7E)) {
			state = st_name;
			name_start = xget(m) - 1;
			break;
		}

		// CR
		if(c == '\r') {
			state = st_init_r;
			break;
		}

		// LF
		if(c == '\n')
			goto endline;

		set_error("Unexpected character '%c' at start of header line", c);
		goto abort;
	case st_init_r:

		// LF
		if(c == '\n')
			goto endline;

		set_error("Unexpected character '%c' after carriage return", c);
		goto abort;
	case st_name:

		// A-NOTCOLON
		if((c >= 0x21 && c <= 0x39) || (c >= 0x3B && c <= 0x7E)) {
			break;
		}

		// COLON
		if(c == ':') {
			name_stop = xget(m) - 1;
			state = st_colon;
			break;
		}

		set_error("Unexpected character '%c' in header name", c);
		goto abort;
	case st_colon:

		// SP
		if(c == ' ') {
			content_start = xget(m);
			state = st_content;
			break;
		}

		// CR
		if(c == '\r') {
			state = st_colon_r;
			break;
		}

		// NL
		if(c == '\n') {
			state = st_colon_n;
			break;
		}

		set_error("Unexpected character '%c' after colon", c);
		goto abort;
	case st_colon_r:

		// NL
		if(c == '\n') {
			state = st_colon_n;
			break;
		}

		set_error("Unexpected character '%c' after carriage return", c);
		goto abort;
	case st_colon_n:

		// SP
		if(c == ' ') {
			content_start = content_stop = xget(m);
			state = st_content;
			break;
		}

		set_error("Unexpected character '%c' after newline", c);
		goto abort;
	case st_content:

		// S-CHAR
		if(c >= 0x21 && c <= 0xFF) {
			break;
		}

		if(c == '\r') {
			content_stop = xget(m) - 1;
			state = st_content_r;
			break;
		}

		if(c == '\n') {
			content_stop = xget(m) - 1;
			state = st_content_n;
			break;
		}

		set_error("Unexpected character '%c' in header content", c);
		goto abort;
	case st_content_r:

		if(c == '\n') {
			state = st_content_n;
			break;
		}

		set_error("Unexpected character '%c' after carriage return", c);
		goto abort;
	case st_content_n:

		if(c == ' ' || c == '\t') {
			state = st_content;
			break;
		}

		xset(m, xget(m) - 1);

		set_error("Unexpected character '%c' after newline", c);
		goto ret;
	}
	set_error("Unexpected end-of-line", c);
abort:
	xset(m, start);
	return 0;

endline:
	return 2;

ret:
	h_append(m, name_start, name_stop, content_start, content_stop);
	return 1;
}

void h_append(snakk_msg *m, size_t name_start, size_t name_end, size_t content_start, size_t content_end)
{
	msg_header *h;
	char *contentraw, *content;
	size_t contentlen;
	size_t i, j;

	m->hn++;
	m->hs = realloc(m->hs, m->hn * sizeof(msg_header));

	h = &(m->hs[m->hn - 1]);

	h->name = strndup(m->src + name_start, name_end - name_start);
	h->namelen = name_end - name_start;

	contentraw = m->src + content_start;
	contentlen = content_end - content_start;

	for(i = 0; i < content_end - content_start; i++)
	switch(contentraw[i]) {
	case '\r':
	case '\n':
		contentlen--;
		break;
	}

	content = malloc(contentlen + 1);

	for(i = 0, j = 0; i < content_end - content_start; i++)
	switch(contentraw[i]) {
	case '\r':
	case '\n':
		break;
	default:
		content[j++] = contentraw[i];
		break;
	}

	content[j] = '\0';

	h->content = content;
	h->contentlen = contentlen;
}

void parsebody(snakk_msg *m)
{
	size_t start, end;
	char buf[512];
	char *body = NULL;

	start = xget(m);
	while(xread(m, buf, sizeof(buf)));
	end = xget(m);

	body = calloc(1, end - start + 1);
	memcpy(body, m->src + start, end - start);
	m->body = body;
	m->bodylen = end - start;
}

int parse(snakk_msg *m)
{
	int ok;

	ok = parsesignature(m);
	if(!ok) {
		set_error("Error parsing signature: %s", snakk_error());
		return 0;
	}
	while((ok = parseheader(m)) == 1);
	if(!ok) {
		set_error("Error while parsing header: %s", snakk_error());
		return 0;
	}
	parsebody(m);

	return 1;
}

snakk_msg *snakk_msg_parse(snakk_msg_readfn readfn, void *rdata)
{
	snakk_msg *msg;
	int ok;

	msg = snakk_msg_create();
	msg->readfn = readfn;
	msg->rdata = rdata;
	ok = parse(msg);

	if(!ok) {
	 	// cleanup(msg);
		return NULL;
	}

	msg->compiled = 1;

	return msg;
}

snakk_msg *snakk_msg_open(const char *path)
{
	FILE *file;
	snakk_msg *m;

	file = fopen(path, "r");
	if(file == NULL) {
		set_error("Failed to open file: %s", strerror(errno));
		return NULL;
	}

	m = snakk_msg_parse((snakk_msg_readfn)fread, file);

	return m;
}

size_t snakk_msg_print(snakk_msg *msg, snakk_msg_writefn writefn, void *wdata);

const char *snakk_msg_header_by_idx_get_content(snakk_msg *msg, int idx)
{
	if(idx < 0 || idx >= msg->hn)
		return NULL;
	return msg->hs[idx].content;
}

const char *snakk_msg_header_by_idx_get_name(snakk_msg *msg, int idx)
{
	if(idx < 0 || idx >= msg->hn)
		return NULL;
	return msg->hs[idx].name;
}

const char *snakk_msg_header_by_name_get_idx(snakk_msg *msg, const char *name);

int snakk_msg_header_get_length(snakk_msg *msg)
{
	return msg->hn;
}

size_t snakk_msg_body_print(snakk_msg *msg, snakk_msg_writefn writefn, void *wdata)
{
	if(msg->compiled)
		return writefn(msg->body, msg->bodylen, 1, wdata);
	return 0;
}

void snakk_msg_destroy(snakk_msg *msg);

const char *snakk_error(void)
{
	return error_string;
}

void set_error(const char *fmt, ...)
{
	va_list ap;
	char cpy[sizeof(error_string)];

	va_start(ap, fmt);
	vsnprintf(cpy, sizeof(error_string), fmt, ap);
	memcpy(error_string, cpy, sizeof(error_string));
	va_end(ap);
}
