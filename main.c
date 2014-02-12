#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "main-extract.h"
#include "main-envelope.h"

static void parseargs(int argc, char *argv[]);
static void usage(FILE *out);
static void version(FILE *out);

struct command {
	char *name;
	int (*main)(int argc, char *argv[]);
};

static struct command commands[] = {
	{ "extract", extract_main },
	{ "envelope", envelope_main },
};

void usage(FILE *out)
{
	fprintf(out, "usage: snakk <file>\n");
}

void version(FILE *out)
{
	puts("snakk version " PACKAGE_VERSION);
}

void parseargs(int argc, char *argv[])
{
	int c;
	static int versionset;

	static struct option longopts[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "version", no_argument, &versionset, 1 },
		{ 0, 0, 0, 0},
	};

	while((c = getopt_long(argc, argv, "h", longopts, NULL)) != -1)
	switch(c) {
	case 'h':
		usage(stdout);
		exit(EXIT_SUCCESS);
	case 0:
		if(versionset) {
			version(stdout);
			exit(EXIT_SUCCESS);
		}
		break;
	default:
		usage(stderr);
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char *argv[])
{
	int i, cmdc;
	char **cmdv = NULL;

	for(i = 1; i < argc; i++)
		if(argv[i][0] != '-')
			break;

	if(i < argc && i > 0) {
		cmdv = &(argv[i]);
		cmdc = argc - i;
	} else {
		parseargs(argc, argv);
		exit(EXIT_FAILURE);
	}

	for(i = 0; i < sizeof(commands) / sizeof(struct command); i++)
		if(!strcmp(cmdv[0], commands[i].name))
			return commands[i].main(cmdc, cmdv);

	printf("Unrecognized snakk command '%s'\n", cmdv[0]);
	exit(EXIT_FAILURE);
}
