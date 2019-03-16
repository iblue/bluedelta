#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "diff.h"

int diff(char* file1name, char *file2name, char *outfilename, int chunksize, int verbose) {
	FILE *file1 = fopen(file1name, "rb");
	if(!file1) {
		perror(file1name);
		exit(2);
	}

	FILE *file2 = fopen(file2name, "rb");
	if(!file2) {
		perror(file2name);
		exit(2);
	}

	FILE *outfile = fopen(outfilename, "wb");
	if(!outfile) {
		perror(outfilename);
		exit(2);
	}

	char* buffer1 = malloc(chunksize); size_t readbytes1=0;
	char* buffer2 = malloc(chunksize); size_t readbytes2=0;
	size_t pos=0;

	// FIXME: Write file header. Should contain:
	// - magic number
	// - version
	// - incomplete flag (until file is done)
	// - size of file1
	// - size of file2
	// - hash of file1 (to check if input file is the correct one)
	// - hash of file2 (to check if it all went well)

	while(1) {
          readbytes1 = fread(buffer1, 1, chunksize, file1);
          readbytes2 = fread(buffer2, 1, chunksize, file2);

	  // We want to produce file2 from file1. So if file2 ended, just write the ending sector and exit.
	  if(readbytes2 < readbytes1) {
	    char cmd = 'E';
            fwrite(&cmd, sizeof(char), 1, outfile);
	    fwrite(&pos, sizeof(size_t), 1, outfile);
	    fwrite(&readbytes2, sizeof(size_t), 1, outfile);
	    fwrite(buffer2, chunksize, 1, outfile);
            break;
	  }

	  // So, if file1 ends, we need to append the rest of file2. We first save an ending record with the last sector
	  if(readbytes1 < readbytes2) {
	    char cmd = 'e';
            fwrite(&cmd, sizeof(char), 1, outfile);
	    fwrite(&pos, sizeof(size_t), 1, outfile);
	    fwrite(&readbytes2, sizeof(size_t), 1, outfile);
	    fwrite(buffer2, chunksize, 1, outfile);
            break;
	  }

	  // Otherwise

	  if(memcmp(buffer1, buffer2, chunksize) != 0) {
	    char cmd = 'd';
            fwrite(&cmd, sizeof(char), 1, outfile);
		  fwrite(&pos, sizeof(size_t), 1, outfile);
		  fwrite(buffer2, chunksize, 1, outfile);

		  if(verbose) {
		    printf("  Found difference at %ld\n", pos);
		  }
	  }
	  pos += chunksize;

	  if(readbytes1 < chunksize) {
		  break;
	  }
	}

	// Append the rest of file2
	while(1) {
          readbytes2 = fread(buffer2, 1, chunksize, file2);
	  fwrite(buffer2, readbytes2, 1, outfile);
	  if(readbytes2 < chunksize) {
		  break;
	  }
	}

	fclose(outfile);
	fclose(file2);
	fclose(file1);

	return 0;

}

