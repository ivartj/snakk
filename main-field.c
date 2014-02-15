#include "main.h"
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include "msg.h"
#include <errno.h>
#include <string.h>
#include "parser.h"
#include "printer.h"

enum type {
	t_first,
	t_all,
};

struct {
	char **fields;
	int *types;
	size_t len, cap;
} field_list = { 0 };

char *delimiter = "\t";

void addfield(const char *name, enum type t)
{
	if(field_list.cap == field_list.len) {
		if(field_list.cap == 0)
			field_list.cap = 256;
		else
			field_list.cap *= 2;
		field_list.fields = realloc(field_list.fields, sizeof(const char *) * field_list.cap);
		field_list.types = realloc(field_list.types, sizeof(enum type) * field_list.cap);
	}
	field_list.types[field_list.len];
	field_list.fields[field_list.len++];
}

void usage(FILE *out)
{
	fprintf(out, "snakk field\n"
	             "	 -f <field-name> ...\n"
	             "        <filename> ...\n");
}

void parseargs(int argc, char *argv[])
{
	int c;
	static struct option longopts[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "first-of", required_argument, NULL, 'f' },
		{ "all-of", required_argument, NULL, 'a' },
		{ "delimiter", required_argument, NULL, 'd' },
		{ 0, 0, 0, 0 },
	};

	while((c = getopt_long(argc, argv, "hf:a:", longopts, NULL)) != -1)
	switch(c) {
	case 'h':
		usage(stdout);
		exit(EXIT_SUCCESS);
	case 'f':
		addfield(optarg, t_first);
		break;
	case 'a':
		addfield(optarg, t_all);
		break;
	case 'd':
		delimiter = optarg;
		break;
	default:
		usage(stderr);
		exit(EXIT_FAILURE);
	}
}

int field_main(int argc, char *argv[])
{

}
