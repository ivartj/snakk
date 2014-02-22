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

static void parseargs(int argc, char *argv[]);
static void openoutput(void);
static void process(void);
static void print(printer *pr, header *hd, int f);

static struct {
	char **fields;
	char **values;
	int *types;
	size_t len, cap;
} field_list = { 0 };

static char *delim = "\t";
static char *placeholder = "_";
static char **filenames = NULL;
static int nfilenames = 0;
static char *outfilename = NULL;
static FILE *outfile = NULL;

void field_usage(FILE *out)
{
	fprintf(out, "snakk field\n"
	             "	 -f <field-name> ...\n"
	             "	 -a <field-name> ...\n"
	             "	 -o <output-file>\n"
	             "	 -d <delimiter>\n"
	             "	 -p <placeholder>\n"
	             "        <filename> ...\n");
}

void addfield(char *name, enum type t)
{
	if(field_list.cap == field_list.len) {
		if(field_list.cap == 0)
			field_list.cap = 256;
		else
			field_list.cap *= 2;
		field_list.fields = realloc(field_list.fields, sizeof(const char *) * field_list.cap);
		field_list.values = realloc(field_list.values, sizeof(const char *) * field_list.cap);
		field_list.types = realloc(field_list.types, sizeof(enum type) * field_list.cap);
	}
	field_list.fields[field_list.len] = name;
	field_list.types[field_list.len] = t;
	field_list.len++;
}

void parseargs(int argc, char *argv[])
{
	int c;
	static struct option longopts[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "output", required_argument, NULL, 'o' },
		{ "first-of", required_argument, NULL, 'f' },
		{ "all-of", required_argument, NULL, 'a' },
		{ "delimiter", required_argument, NULL, 'd' },
		{ "placeholder", required_argument, NULL, 'p' },
		{ 0, 0, 0, 0 },
	};

	while((c = getopt_long(argc, argv, "hf:a:", longopts, NULL)) != -1)
	switch(c) {
	case 'h':
		field_usage(stdout);
		exit(EXIT_SUCCESS);
	case 'o':
		outfilename = optarg;
		break;
	case 'f':
		addfield(optarg, t_first);
		break;
	case 'a':
		addfield(optarg, t_all);
		break;
	case 'd':
		delim = optarg;
		break;
	case 'p':
		placeholder = optarg;
		break;
	default:
		field_usage(stderr);
		exit(EXIT_FAILURE);
	}

	switch(argc - optind) {
	case 0:
		field_usage(stderr);
		exit(EXIT_FAILURE);
	default:
		filenames = &(argv[optind]);
		nfilenames = argc - optind;
		break;
	}
}

void openoutput(void)
{
	if(outfilename == NULL)
		outfile = stdout;
	else {
		outfile = fopen(outfilename, "w");
		if(outfile == NULL) 
			die_errno("Failed to open '%s'", outfilename);
	}
}

void openfile(parser *p, const char *filename)
{
	FILE *in;

	in = fopen(filename, "rb");
	if(in == NULL)
		die_errno("Failed to open '%s'", filename);

	parser_init(p, (parser_readfn)fread, (void *)in);
}

void print(printer *pr, header *hd, int f)
{
	int i;
	char *value;
	field *fld;
	int match;

	if(f == field_list.len) {
		for(i = 0; i < field_list.len; i++) {
			if(field_list.values[i] == NULL)
				fprintf(outfile, "%s", placeholder);
			else
				fprintf(outfile, "%s", field_list.values[i]);
			if(i + 1 < field_list.len)
				fprintf(outfile, "%s", delim);
		}
		fprintf(outfile, "\n");
		return;
	}

	switch(field_list.types[f]) {
	case t_first:
		value = header_get(hd, field_list.fields[f]);
		if(value == NULL)
			value = NULL;
		field_list.values[f] = value;
		print(pr, hd, f + 1);
		break;
	case t_all:
		match = 0;
		for(i = 0; i < header_get_field_num(hd); i++) {
			fld = header_get_field_i(hd, i);
			if(!strcmp(field_list.fields[f], field_get_name(fld))) {
				match = 1;
				field_list.values[f] = field_get_value(fld);
				print(pr, hd, f + 1);
			}
		}
		if(!match) {
			field_list.values[f] = NULL;
			print(pr, hd, f + 1);
		}
		break;
	}
}

void closefile(parser *p)
{
	fclose((FILE *)(p->rdata));
	parser_reset(p);
}

void process(void)
{
	int i;
	parser p;
	printer pr;
	FILE *in;
	header *hd;

	printer_init(&pr, (printer_writefn)fwrite, (void *)outfile);

	for(i = 0; i < nfilenames; i++) {
		openfile(&p, filenames[i]);
		hd = parse_header(&p);
		if(hd == NULL) {
			fprintf(stderr, "Failed to parse header in file '%s': %s.\n", filenames[i], p.errmsg);
			exit(EXIT_FAILURE);
		}
		print(&pr, hd, 0);
		closefile(&p);
	}
}

int field_main(int argc, char *argv[])
{
	parseargs(argc, argv);
	openoutput();
	process();
	exit(EXIT_SUCCESS);
}
