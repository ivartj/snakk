#ifndef MSG_H
#define MSG_H

#include <stdio.h>

typedef struct _msg msg;
typedef struct _header header;
typedef struct _field field;

/* ############### *
 * ### MESSAGE ### *
 * ############### */

msg *msg_create(void);
void msg_set_header(msg *m, header *hd);
void msg_set_src_on(msg *m, char *src, size_t off, size_t len);
void msg_set_body(msg *m, char *body);
void msg_set_body_n(msg *m, char *body, size_t bodylen);

header *msg_get_header(msg *m);
char *msg_get_src(msg *m);
char *msg_get_src_n(msg *m, size_t *rlen);
char *msg_get_body(msg *m);
char *msg_get_body_n(msg *m, size_t *rlen);
char *msg_get_src_n(msg *m, size_t *rlen);

void msg_destroy(msg *m);

/* ############## *
 * ### HEADER ### *
 * ############## */

header *header_create(void);
void header_set_src_on(header *m, char *src, size_t off, size_t len);
void header_add_field(header *hd, field *f);
void header_prepend_field(header *hd, field *f);

void header_set_top_i(header *hd, int idx);
int header_get_index(header *hd, char *name);

char *header_get_src_n(header *m, size_t *rlen);
char *header_get(header *hd, char *name);
char *header_get_n(header *hd, char *name, size_t namelen, size_t *rvaluelen);
field *header_get_field(header *hd, char *name);
field *header_get_field_i(header *hd, int idx);
int header_get_field_num(header *hd);

void header_destroy(header *hd);

/* ############# *
 * ### FIELD ### *
 * ############# */

field *field_create(void);
void field_set_src_on(field *m, char *src, size_t off, size_t len);
void field_set_name(field *f, char *name);
void field_set_name_n(field *f, char *name, size_t namelen);
void field_set_value(field *f, char *value);
void field_set_value_n(field *f, char *value, size_t valuelen);

char *field_get_src_n(field *m, size_t *rlen);
char *field_get_name(field *f);
char *field_get_name_n(field *f, size_t *rlen);
char *field_get_value(field *f);
char *field_get_value_n(field *f, size_t *rlen);

void field_destroy(field *f);

#endif
