#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "diff.h"

int patch(char* file1name, char *file2name, char *outfilename, int chunksize, int verbose) {
	FILE *file1 = fopen(file1name, "rb");
	if(!file1) {
		perror(file1name);
		exit(2);
	}

	FILE *file2 = fopen(file2name, "rw");
	if(!file2) {
		perror(file2name);
		exit(2);
	}

	FILE *outfile = fopen(outfilename, "rb");
	if(!outfile) {
		perror(outfilename);
		exit(2);
	}

	// FIXME: Get from file header
	/*
	char* buffer = malloc(chunksize);
	
	size_t readbytes1=0;
*/
	return 0;
}

