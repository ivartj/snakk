#include <stdlib.h>
#include <stdio.h>
#include "msg.h"

int main(int argc, char *argv[])
{
	snakk_msg *m;
	int i;

	m = snakk_msg_open("test.txt");
	if(m == NULL) {
		fprintf(stderr, "snakk_msg_open: %s\n", snakk_error());
		exit(EXIT_FAILURE);
	}

	for(i = 0; i < snakk_msg_header_get_length(m); i++)
		printf("[%s|%s]\n", snakk_msg_header_by_idx_get_name(m, i), snakk_msg_header_by_idx_get_content(m, i));
	snakk_msg_body_print(m, (snakk_msg_writefn)fwrite, stdout);

	exit(EXIT_SUCCESS);
}
