#include "main.h"
#include "parser.h"
#include "printer.h"
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

static void parseargs(int argc, char *argv[]);
static void openfiles(void);
static void parse(void);
static void print(void);

static char *outfilename = NULL;
static FILE *outfile = NULL;

static char **filenames = NULL;
static int nfilenames = 0;
static msg *m = NULL;

static enum {
	format_verbatim,
	format_unfolded,
} format = format_verbatim;

void envelope_usage(FILE *out)
{
	fprintf(out,
	"snakk envelope <file1> <file2> ...\n"
	"	[ -o | --output=<file> ]\n"
	"\n"
	"Envelopes file1 in file2, then file3 and so on.\n");
}

void parseargs(int argc, char *argv[])
{
	int c;
	static struct option longopts[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "output", required_argument, NULL, 'o' },
		{ "format", required_argument, NULL, 'f' },
		{ "envelope", required_argument, NULL, 'e' },
		{ 0, 0, 0, 0 },
	};

	while((c = getopt_long(argc, argv, "ho:e:f:", longopts, NULL)) != -1)
	switch(c) {
	case 'h':
		envelope_usage(stdout);
		exit(EXIT_SUCCESS);
	case 'o':
		outfilename = optarg;
		break;
	case 'f':
		if(strcmp(optarg, "verbatim") == 0) {
			format = format_verbatim;
			break;
		}
		if(strcmp(optarg, "unfolded") == 0) {
			format = format_unfolded;
			break;
		}
		die("Unrecognized format '%s'", optarg);
	default:
		envelope_usage(stderr);
		exit(EXIT_FAILURE);
	}

	switch(argc - optind) {
	case 0:
		envelope_usage(stderr);
		exit(EXIT_FAILURE);
	default:
		filenames = &(argv[optind]);
		nfilenames = argc - optind;
		break;
	}
}

void openoutput(void)
{
	if(outfilename != NULL) {
		outfile = fopen(outfilename, "w");
		if(outfile == NULL)
			die_errno("Failed to open file '%s'", outfilename);
	} else
		outfile = stdout;
}

void parse(void)
{
	parser p;
	int i, j, idx;
	FILE *file;
	field *fld;
	header *hd;

	file = fopen(filenames[0], "rb");
	if(file == NULL)
		die_errno("Failed to open file '%s'", outfilename);

	parser_init(&p, (parser_readfn)fread, (void *)file);
	m = parse_msg(&p);
	if(m == NULL)
		die("Failed to parse file '%s'", filenames[0]);

	hd = msg_get_header(m);

	for(i = 1; i < nfilenames; i++) {
		file = fopen(filenames[i], "rb");
		if(file == NULL)
			die_errno("Failed to open file '%s'", filenames[i]);
		parser_init(&p, (parser_readfn)fread, (void *)file);
		j = 0;
		while((fld = parse_field(&p)) != NULL) {
			if((idx = header_get_index(hd, field_get_name(fld))) == -1)
				header_insert_field(hd, fld, j);
			else {
				field_destroy(fld);
				header_move_field(hd, idx, j);
			}
			j++;
		}
	}
}

void print(void)
{
	int i;
	header *hd;
	field *fld;
	printer pr;
	char *body;
	size_t bodylen;

	printer_init(&pr, (printer_writefn)fwrite, outfile);

	hd = msg_get_header(m);
	for(i = 0; i < header_get_field_num(hd); i++) {
		fld = header_get_field_i(hd, i);

		switch(format) {
		case format_verbatim:
			print_field_verbatim(&pr, fld);
			break;
		case format_unfolded:
			print_field_verbatim(&pr, fld);
			break;
		}
	}

	body = msg_get_body_n(m, &bodylen);
	xprintf(&pr, "\r\n");
	print_chunk(&pr, body, bodylen);
}

int envelope_main(int argc, char *argv[])
{
	parseargs(argc, argv);
	openoutput();
	parse();
	print();
	exit(EXIT_SUCCESS);
}
