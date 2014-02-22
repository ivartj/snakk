#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdio.h>

int dir_isdir(const char *path);

void dir_conf_set(const char *path);
void dir_data_set(const char *path);

const char *dir_conf_get(void);
const char *dir_data_get(void);

FILE *dir_conf_open(const char *path, const char *mode);
FILE *dir_data_open(const char *path, const char *mode);

#endif
