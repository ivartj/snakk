#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>

void die(const char *fmt, ...);
void die_errno(const char *fmt, ...);

int  envelope_main(int argc, char *argv[]);
void envelope_usage(FILE *out);

int  extract_main(int argc, char *argv[]);
void extract_usage(FILE *out);

int  header_main(int argc, char *argv[]);
void header_usage(FILE *out);

int  field_main(int argc, char *argv[]);
void field_usage(FILE *out);

#endif
