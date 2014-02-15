#include "main.h"
#include "parser.h"
#include "printer.h"
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

static void parseargs(int argc, char *argv[]);
static FILE *openfile_w(const char *filename);
static FILE *openfile_r(const char *filename);
static void parse(void);
static void print(printer *pr);

static enum {
	format_verbatim,
	format_unfolded,
} format = format_verbatim;

int nfiles = 0;
char **filenames = NULL;
char *outfilename = NULL;

FILE *file = NULL;
FILE *outfile = NULL;

header *hd = NULL;
char *verbatim = NULL;
size_t verbatimlen = 0;

void header_usage(FILE *out)
{
	fprintf(out, "usage: snakk header [ <file> ]\n"
	             "	[ -o <output-file> ]\n"
	             "	[ -f | --format= { verbatim | unfolded } ]\n");
}

void parseargs(int argc, char *argv[])
{
	int c;
	static struct option longopts[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "output", required_argument, NULL, 'o' },
		{ "format", required_argument, NULL, 'f' },
		{ 0, 0, 0, 0 },
	};

	while((c = getopt_long(argc, argv, "ho:", longopts, NULL)) != -1)
	switch(c) {
	case 'h':
		header_usage(stdout);
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
		fprintf(stderr, "Unrecognized format '%s'.\n", optarg);
		exit(EXIT_FAILURE);
	case '?':
		header_usage(stderr);
		exit(EXIT_FAILURE);
	}

	switch(argc - optind) {
	case 0:
		break;
	default:
		filenames = &(argv[optind]);
		nfiles = argc - optind;
		break;
	}
}

FILE *openfile_w(const char *filename)
{
	FILE *file;
	file = fopen(filename, "w");
	if(file == NULL) {
		fprintf(stderr, "Failed to open file '%s': %s.\n", filename, strerror(errno));
		exit(EXIT_FAILURE);
	}
	return file;
}

FILE *openfile_r(const char *filename)
{
	FILE *file;
	file = fopen(filename, "rb");
	if(file == NULL) {
		fprintf(stderr, "Failed to open file '%s': %s.\n", filename, strerror(errno));
		exit(EXIT_FAILURE);
	}
	return file;
}

void print(printer *p)
{
	switch(format) {
	case format_verbatim:
		print_chunk(p, verbatim, verbatimlen);
		break;
	case format_unfolded:
		print_header(p, hd);
		break;
	}
}

void parse(void)
{
	parser p;
	printer pr;
	int i;

	printer_init(&pr, (printer_writefn)fwrite, (void *)outfile);

	if(nfiles == 0) {
		parser_init(&p, (parser_readfn)fread, (void *)stdin);
		hd = parse_header(&p);
		if(hd == NULL) {
			fprintf(stderr, "Failed to parse file: %s.\n", p.errmsg);
			exit(EXIT_FAILURE);
		}

		verbatim = xcpy(&p, 0, xget(&p));
		verbatimlen = xget(&p);
		print(&pr);

	} else for(i = 0; i < nfiles; i++) {
		file = openfile_r(filenames[i]);
		parser_init(&p, (parser_readfn)fread, (void *)file);
		hd = parse_header(&p);
		if(hd == NULL) {
			fprintf(stderr, "Failed to parse file: %s.\n", p.errmsg);
			exit(EXIT_FAILURE);
		}

		verbatim = xcpy(&p, 0, xget(&p));
		verbatimlen = xget(&p);
		print(&pr);
		parser_reset(&p);
		if(i + 1 < nfiles)
			xprintf(&pr, "\r\n");
	}
}

int header_main(int argc, char *argv[])
{
	parseargs(argc, argv);
	if(outfilename == NULL)
		outfile = stdout;
	else
		outfile = openfile_w(outfilename);
	parse();
	exit(EXIT_SUCCESS);
}
