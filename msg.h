#ifndef SNAKK_MSG_H
#define SNAKK_MSG_H

#include <stdio.h>

typedef struct _snakk_msg snakk_msg;

typedef size_t (*snakk_msg_readfn)(void *ptr, size_t size, size_t nitems, void *rdata);
typedef size_t (*snakk_msg_writefn)(const void *ptr, size_t size, size_t nitems, void *wdata);

const char *snakk_error(void);

snakk_msg *snakk_msg_open(const char *path);
snakk_msg *snakk_msg_parse(snakk_msg_readfn readfn, void *rdata);
size_t snakk_msg_print(snakk_msg *msg, snakk_msg_writefn writefn, void *wdata);
snakk_msg *snakk_msg_create(void);

const char *snakk_msg_header_by_idx_get_content(snakk_msg *msg, int idx);
const char *snakk_msg_header_by_idx_get_name(snakk_msg *msg, int idx);
const char *snakk_msg_header_by_name_get_idx(snakk_msg *msg, const char *name);
int snakk_msg_header_get_length(snakk_msg *msg);

size_t snakk_msg_body_print(snakk_msg *msg, snakk_msg_writefn writefn, void *wdata);

void snakk_msg_destroy(snakk_msg *msg);

#endif
