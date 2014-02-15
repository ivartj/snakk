#include "main.h"
#include "msg.h"
#include "parser.h"
#include "printer.h"
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static void parseargs(int argc, char *argv[]);
static void openfiles(void);
static void parse(void);
static void extract(void);

static FILE *file = NULL;
static char *filename = NULL;
static FILE *outfile = NULL;
static char *outfilename = NULL;
static msg *m = NULL;

void extract_usage(FILE *out)
{
	fprintf(out, "snakk extract [ <file> ] [ -o <output-file> ]\n");
}

void parseargs(int argc, char *argv[])
{
	int c;
	static struct option longopts[] = {
		{ "output", required_argument, NULL, 'o' },
		{ "help", no_argument, NULL, 'h' },
		{ 0, 0, 0, 0 },
	};

	while((c = getopt_long(argc, argv, "o", longopts, NULL)) != -1) {
		switch(c) {
		case 'o':
			outfilename = optarg;
			break;
		case 'h':
			extract_usage(stdout);
			exit(EXIT_SUCCESS);
		case '?':
			extract_usage(stderr);
			exit(EXIT_FAILURE);
		}
	}

	switch(argc - optind) {
	case 0:
		break;
	case 1:
		filename = argv[optind];
		break;
	default:
		extract_usage(stderr);
		exit(EXIT_FAILURE);
	}
}

void openfiles(void)
{
	if(filename != NULL) {
		file = fopen(filename, "rb");
		if(file == NULL) {
			fprintf(stderr, "Failed to open file '%s':\n\t%s.\n", filename, strerror(errno));
			exit(EXIT_FAILURE);
		}
	} else
		file = stdin;

	if(outfilename != NULL) {
		outfile = fopen(outfilename, "w");
		if(outfile == NULL) {
			fprintf(stderr, "Failed to open file '%s':\n\t%s.\n", outfilename, strerror(errno));
			exit(EXIT_FAILURE);
		}
	} else
		outfile = stdout;
}

void parse(void)
{
	parser p;

	parser_init(&p, (parser_readfn)fread, (void *)file);
	m = parse_msg(&p);
	if(m == NULL) {
		fprintf(stderr, "Failed to parse message: %s.\n", p.errmsg);
		exit(EXIT_FAILURE);
	}
}

void extract(void)
{
	printer p;
	char *body;
	size_t bodylen;

	body = msg_get_body_n(m, &bodylen);
	printer_init(&p, (printer_writefn)fwrite, (void *)outfile);
	print_chunk(&p, body, bodylen);
}

int extract_main(int argc, char *argv[])
{
	parseargs(argc, argv);
	openfiles();
	parse();
	extract();

	exit(EXIT_SUCCESS);
}
