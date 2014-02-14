#include "msg.h"
#include <stdlib.h>
#include <string.h>

struct _msg {
	char *src;
	size_t srcoff;
	size_t srclen;
	header *hd;
	char *body;
	size_t bodylen;
};

struct _header {
	char *src;
	size_t srcoff;
	size_t srclen;
	field **fieldv;
	int fieldc;
};

struct _field {
	char *src;
	size_t srcoff;
	size_t srclen;
	char *name, *value;
	size_t namelen, valuelen;
};


/* ############### *
 * ### MESSAGE ### *
 * ############### */

msg *msg_create(void)
{
	return calloc(1, sizeof(msg));
}

void msg_set_header(msg *m, header *hd)
{
	m->hd = hd;
}

void msg_set_src_on(msg *m, char *src, size_t off, size_t stop)
{
	m->src = src + off;
	m->srclen = stop - off;
}

void msg_set_body(msg *m, char *body)
{
	m->body = body;
	m->bodylen = strlen(body);
}

void msg_set_body_n(msg *m, char *body, size_t bodylen)
{
	m->body = body;
	m->bodylen = bodylen;
}

header *msg_get_header(msg *m)
{
	return m->hd;
}

char *msg_get_src(msg *m)
{
	return m->src;
}

char *msg_get_src_n(msg *m, size_t *rlen)
{
	if(rlen != NULL)
		*rlen = m->srclen;
	return m->src;
}

char *msg_get_body(msg *m)
{
	return m->body;
}

char *msg_get_body_n(msg *m, size_t *rlen)
{
	if(rlen != NULL)
		*rlen = m->bodylen;
	return m->body;
}

void msg_destroy(msg *m)
{
	int i;

	if(m->hd != NULL)
		header_destroy(m->hd);

	if(m->body != NULL)
		free(m->body);

	free(m);
}

/* ############## *
 * ### HEADER ### *
 * ############## */

header *header_create(void)
{
	return calloc(1, sizeof(header));
}

void header_set_src_on(header *hd, char *src, size_t off, size_t stop)
{
	hd->src = src + off;
	hd->srclen = stop - off;
}

void header_add_field(header *hd, field *f)
{
	hd->fieldc++;
	hd->fieldv = realloc(hd->fieldv, sizeof(field *) * hd->fieldc);
	hd->fieldv[hd->fieldc - 1] = f;
}

void header_move_field(header *hd, int idx, int to)
{
	field *f;
	int i;

	f = hd->fieldv[idx];

	for(i = idx; i > to; i--)
		hd->fieldv[i] = hd->fieldv[i - 1];
	for(i = idx; i < to; i++)
		hd->fieldv[i] = hd->fieldv[i + 1];

	hd->fieldv[to] = f;
}

void header_insert_field(header *hd, field *f, int idx)
{
	hd->fieldc++;
	hd->fieldv = realloc(hd->fieldv, sizeof(field *) * hd->fieldc);
	memmove(&(hd->fieldv[1 + idx]), &(hd->fieldv[idx]), sizeof(field *) * (hd->fieldc - 1 - idx));
	hd->fieldv[idx] = f;
}

int header_get_index(header *hd, char *name)
{
	int i;
	size_t namelen;

	namelen = strlen(name);

	for(i = 0; i < hd->fieldc; i++)
		if(hd->fieldv[i]->namelen == namelen)
		if(!strncmp(hd->fieldv[i]->name, name, namelen))
			break;

	if(i == hd->fieldc)
		return -1;

	return i;
}

void header_set_top_i(header *hd, int idx)
{
	field *f;
	int i;

	f = hd->fieldv[idx];

	for(i = idx; i > 0; i--)
		hd->fieldv[i] = hd->fieldv[i - 1];
	hd->fieldv[0] = f;
}

char *header_get(header *hd, char *name)
{
	field *f;
	f = header_get_field(hd, name);
	if(f == NULL)
		return NULL;
	return f->value;
}

field *header_get_field(header *hd, char *name)
{
	int i;
	size_t namelen;

	namelen = strlen(name);

	for(i = 0; i < hd->fieldc; i++)
		if(hd->fieldv[i]->namelen == namelen)
		if(!strncmp(hd->fieldv[i]->name, name, namelen))
			break;

	if(i == hd->fieldc)
		return NULL;

	return hd->fieldv[i];
}

char *header_get_n(header *hd, char *name, size_t namelen, size_t *rvaluelen)
{
	int i;

	for(i = 0; i < hd->fieldc; i++)
		if(hd->fieldv[i]->namelen == namelen)
		if(!strncmp(hd->fieldv[i]->name, name, namelen))
			break;

	if(i == hd->fieldc)
		return NULL;

	if(rvaluelen != NULL)
		*rvaluelen = hd->fieldv[i]->valuelen;

	return hd->fieldv[i]->value;
}

field *header_get_field_i(header *hd, int idx)
{
	if(idx >= hd->fieldc)
		return NULL;
	return hd->fieldv[idx];
}

int header_get_field_num(header *hd)
{
	return hd->fieldc;
}

void header_destroy(header *hd)
{
	int i;

	for(i = 0; i < hd->fieldc; i++)
		field_destroy(hd->fieldv[i]);

	if(hd->fieldv != NULL)
		free(hd->fieldv);

	free(hd);
}

/* ############# *
 * ### FIELD ### *
 * ############# */

field *field_create(void)
{
	return calloc(1, sizeof(field));
}

void field_set_src_on(field *fld, char *src, size_t off, size_t stop)
{
	fld->src = src + off;
	fld->srclen = stop - off;
}

char *field_get_src_n(field *fld, size_t *rlen)
{
	*rlen = fld->srclen;
	return fld->src;
}


void field_set_name(field *f, char *name)
{
	f->name = name;
	f->namelen = strlen(name);
}

void field_set_name_n(field *f, char *name, size_t namelen)
{
	f->name = name;
	f->namelen = namelen;
}

void field_set_value(field *f, char *value)
{
	f->value = value;
	f->valuelen = strlen(value);
}

void field_set_value_n(field *f, char *value, size_t valuelen)
{
	f->value = value;
	f->valuelen = valuelen;
}

char *field_get_name(field *f)
{
	return f->name;
}

char *field_get_name_n(field *f, size_t *rlen)
{
	if(rlen != NULL)
		*rlen = f->namelen;
	return f->name;
}

char *field_get_value(field *f)
{
	return f->value;
}

char *field_get_value_n(field *f, size_t *rlen)
{
	if(rlen != NULL)
		*rlen = f->valuelen;
	return f->value;
}

void field_destroy(field *f)
{
	if(f->name != NULL)
		free(f->name);
	if(f->value != NULL)
		free(f->value);
}
