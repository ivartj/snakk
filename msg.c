#include "msg.h"
#include <stdlib.h>
#include <string.h>

struct _msg {
	header *hd;
	header **sigv;
	int sigc;
	char *body;
	size_t bodylen;
};

struct _header {
	field **fieldv;
	int fieldc;
};

struct _field {
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

void msg_add_sig(msg *m, header *hd)
{
	m->sigc++;
	m->sigv = realloc(m->sigv, sizeof(header *) * m->sigc);
	m->sigv[m->sigc - 1] = hd;
}

int msg_get_sig_num(msg *m)
{
	return m->sigc;
}

header *msg_get_sig_i(msg *m, int idx)
{
	if(idx >= m->sigc)
		return NULL;
	return m->sigv[idx];
}

header *msg_get_header(msg *m)
{
	return m->hd;
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

	if(m->sigc != 0) {

		for(i = 0; i < m->sigc; i++)
			header_destroy(m->sigv[i]);

		free(m->sigv);
	}

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

void header_add_field(header *hd, field *f)
{
	hd->fieldc++;
	hd->fieldv = realloc(hd->fieldv, sizeof(field *) * hd->fieldc);
	hd->fieldv[hd->fieldc - 1] = f;
}

char *header_get(header *hd, char *name)
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

	return hd->fieldv[i]->value;
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
