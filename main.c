#include "parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>

char *filename = NULL;
FILE *file = NULL;

void usage(FILE *out)
{
	fprintf(out, "usage: snakk <file>\n");
}

void parseargs(int argc, char *argv[])
{
	int c;
	static struct option longopts[] = {
		{ "help", no_argument, NULL, 'h' },
		{ 0, 0, 0, 0 },
	};

	while((c = getopt_long(argc, argv, "h", longopts, NULL)) != -1)
	switch(c) {
	case 'h':
		usage(stdout);
		exit(EXIT_SUCCESS);
	case '?':
		usage(stderr);
		exit(EXIT_FAILURE);
	}

	switch(argc - optind) {
	case 1:
		filename = argv[optind];
		break;
	default:
		usage(stderr);
		exit(EXIT_FAILURE);
	}
}

void openfile(void)
{
	file = fopen(filename, "rb");
	if(file == NULL) {
		fprintf(stderr, "Failed to open file: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char *argv[])
{
	msg *m;
	header *hd;
	field *f;
	parser p;
	int i, j;

	parseargs(argc, argv);
	openfile();

	parser_init(&p, (parser_readfn)fread, file);
	m = parse_msg(&p);
	if(m == NULL) {
		fprintf(stderr, "ERROR: %s.\n", p.errmsg);
		exit(EXIT_FAILURE);
	}

	for(i = 0; i < msg_get_sig_num(m); i++) {
		hd = msg_get_sig_i(m, i);
		printf("sig by %s\n\t%s\n\n",
			header_get(hd, "Identity"),
			header_get(hd, "Signature"));
	}

	hd = msg_get_header(m);

	for(i = 0; i < header_get_field_num(hd); i++) {
		f = header_get_field_i(hd, i);
		printf("{%s|%s}\n", field_get_name(f), field_get_value(f));
	}

	printf("%s", msg_get_body(m));

	exit(EXIT_SUCCESS);
}
