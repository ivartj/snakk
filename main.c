#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "main.h"

static void parseargs(int argc, char *argv[]);
static void usage(FILE *out);
static void version(FILE *out);
static int help_main(int argc, char *argv[]);
static int version_main(int argc, char *argv[]);
static void help_usage(FILE *out);
static void version_usage(FILE *out);

struct command {
	char *name;
	int (*main)(int argc, char *argv[]);
	void (*usage)(FILE *out);
};

static struct command commands[] = {
	{ "help", help_main, help_usage },
	{ "version", version_main, version_usage },
	{ "extract", extract_main, extract_usage },
	{ "envelope", envelope_main, envelope_usage },
	{ "header", header_main, header_usage },
	{ "field", field_main, field_usage },
};

void usage(FILE *out)
{
	int i;

	fprintf(out, "snakk <command> <args...>\n");
	for(i = 0; i < sizeof(commands) / sizeof(struct command); i++)
		fprintf(out, "	%s\n", commands[i].name);
}

void version(FILE *out)
{
	puts("snakk version " PACKAGE_VERSION);
}

int help_main(int argc, char *argv[])
{
	int i;

	if(argc >= 2) {
		for(i = 0; i < sizeof(commands) / sizeof(struct command); i++)
		if(strcmp(argv[1], commands[i].name) == 0)
		if(commands[i].usage != NULL) {
			commands[i].usage(stdout);
			break;
		}
	} else
		usage(stdout);

	exit(EXIT_SUCCESS);
}

void help_usage(FILE *out)
{
	fprintf(out, "snakk help [ <command> ]\n");
}

int version_main(int argc, char *argv[])
{
	version(stdout);
	exit(EXIT_SUCCESS);
}

void version_usage(FILE *out)
{
	fprintf(out, "Prints out version.\n");
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
		usage(stderr);
		exit(EXIT_FAILURE);
	}

	for(i = 0; i < sizeof(commands) / sizeof(struct command); i++)
		if(!strcmp(cmdv[0], commands[i].name))
			return commands[i].main(cmdc, cmdv);

	printf("Unrecognized snakk command '%s'\n", cmdv[0]);
	exit(EXIT_FAILURE);
}
