#include "dir.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdarg.h>
#include "buffer.h"
#include "writer.h"

char *confdir = NULL;
char *datadir = NULL;

char *dir_mkpathdup(const char *fmt, ...)
{
	buffer buf;
	writer w;
	va_list ap;
	char *dup;

	va_start(ap, fmt);
	buffer_init(&buf);
	buffer_writer(&buf, &w);
	writer_vprintf(&w, fmt, ap);
	writer_reset(&w);
	dup = strdup(buffer_get(&buf));
	buffer_reset(&buf);
	return dup;
}

int dir_isdir(const char *path)
{
	int err;
	struct stat buf;

	err = stat(path, &buf); 
	if(err == -1)
		return 0;

	if(buf.st_mode & S_IFDIR)
		return 1;

	return 0;
}

void dir_conf_set(const char *path)
{
	if(!dir_isdir(path))
		die("%s is not a directory");
	confdir = strdup(path);
}

void dir_data_set(const char *path)
{
	if(!dir_isdir(path))
		die("%s is not a directory");
	datadir = strdup(path);
}

const char *dir_conf_get(void)
{
	char *home;

	if(confdir != NULL)
		return confdir;

	home = getenv("XDG_CONFIG_HOME");
	if(home != NULL) {
		if(!dir_isdir(home))
			die("XDG_CONFIG_HOME is set to an invalid directory");
		confdir = dir_mkpathdup("%s/%s", home, "snakk");
		return confdir;
	}

	home = getenv("HOME");
	if(home != NULL) {
		if(!dir_isdir(home))
			die("HOME is set to an invalid directory");
		confdir = dir_mkpathdup("%s/.%s/conf", home, "snakk");
		return confdir;
	}

	die("Neither the XDG_CONFIG_HOME nor the HOME environmental variables are defined");
}

const char *dir_data_get(void)
{
	const char *home;

	if(datadir != NULL)
		return datadir;

	home = getenv("XDG_DATA_HOME");
	if(home != NULL) {
		if(!dir_isdir(home))
			die("XDG_DATA_HOME is set to an invalid directory");
		datadir = dir_mkpathdup("%s/%s", home, "snakk");
		return datadir;
	}

	home = getenv("HOME");
	if(home != NULL) {
		if(!dir_isdir(home))
			die("HOME is set to an invalid directory");
		datadir = dir_mkpathdup("%s/.%s/data", home, "snakk");
		return datadir;
	}

	die("Neither the XDG_CONFIG_HOME nor the HOME environmental variables are defined");
}

FILE *dir_conf_open(const char *path, const char *mode)
{
	char *pathdup;
	FILE *file;

	pathdup = dir_mkpathdup("%s/%s", dir_conf_get(), path);
	file = fopen(pathdup, mode);
	if(file == NULL)
		die_errno("Failed to open configration file '%s'", pathdup);
	free(pathdup);
	return file;
}

FILE *dir_data_open(const char *path, const char *mode)
{
	char *pathdup;
	FILE *file;

	pathdup = dir_mkpathdup("%s/%s", dir_data_get(), path);
	file = fopen(pathdup, mode);
	if(file == NULL)
		die_errno("Failed to open internal data file '%s'", pathdup);
	free(pathdup);
	return file;
}

