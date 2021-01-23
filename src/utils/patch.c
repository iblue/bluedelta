#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "format.h"
#include "patch.h"

static inline void copy(FILE* out, FILE* in, size_t bytes, int chunksize) {
	uint8_t* buffer = malloc(chunksize);

	while(1) {
	  int readbytes = bytes;
	  if(readbytes > chunksize) {
		readbytes = chunksize;
	  }
          int bytecnt = fread(buffer, 1, readbytes, in);
          fwrite(buffer, bytecnt, 1, out);
	  if(bytecnt < readbytes) {
		break;
	  }
	  bytes -= bytecnt;
	  if(bytes == 0) {
		break;
	  }
	}

	free(buffer);
}

int patch(char* file1name, char *file2name, char *patchfilename, int chunksize, int verbose) {
	FILE *file1 = fopen(file1name, "rb");
	if(!file1) {
		perror(file1name);
		exit(2);
	}

	FILE *file2 = fopen(file2name, "wb");
	if(!file2) {
		perror(file2name);
		exit(2);
	}

	FILE *patchfile = fopen(patchfilename, "rb");
	if(!patchfile) {
		perror(patchfilename);
		exit(2);
	}

	// Read file header
	if(verbose) {
		printf("  DEBUG: Checking header...\n");
	}
	file_header_t header;
	size_t records = fread(&header, sizeof(file_header_t), 1, patchfile);

	if(records != 1) {
          printf("  ERROR: %s is not valid patch file (fread on header failed)\n", patchfilename);
	  exit(128);
	}

	// Check header
	if(header.magic != HEADER_MAGIC) {
          printf("  ERROR: %s is not valid patch file (header magic incorrect)\n", patchfilename);
	  exit(128);
	}

	if(header.version != HEADER_VERSION) {
          printf("  ERROR: %s is not valid patch file (version mismatch)\n", patchfilename);
	  exit(128);
	}

	if(header.incomplete != HEADER_INCOMPLETE_NO) {
          printf("  ERROR: %s is not valid patch file (file incomplete flag set)\n", patchfilename);
	  exit(128);
	}

	if(header.chunk_size == 0) {
          printf("  ERROR: %s is not valid patch file (invalid chunk size)\n", patchfilename);
	  exit(128);
	}

	// Check input file size
	if(verbose) {
		printf("  DEBUG: Checking input file size...\n");
	}
	fseek(file1, 0, SEEK_END);
	if(header.file1_size != ftell(file1)) {
          printf("  ERROR: %s is not a valid input file (expected to be %ld bytes, is %ld bytes)\n", file2name, (long int)header.file1_size, ftell(file1));
	  exit(128);
	}
	fseek(file1, 0, SEEK_SET);

	data_record_t diff_record;
	uint64_t position=0;
	if(verbose) {
		printf("  DEBUG: File positions:\n");
		printf("    FILE1 (input):  %ld\n", ftell(file1));
		printf("    FILE2 (output): %ld\n", ftell(file2));
		printf("    PATCH FILE:     %ld\n\n", ftell(patchfile));
	}

	while(!feof(patchfile)) {
		records = fread(&diff_record.position, sizeof(diff_record.position), 1, patchfile);
		if(records != 1) {
			if(feof(patchfile)) {
				break;
			}
			printf("  ERROR: Could not read data record position structure (fread failed)\n");
			exit(128);
		}
		records = fread(&diff_record.length, sizeof(diff_record.length), 1, patchfile);
		if(records != 1) {
			printf("  ERROR: Could not read data record position structure (fread failed)\n");
			exit(128);
		}

		if(verbose) {
			printf("  DEBUG: Found diff record. Position: %ld, length: %ld\n", (long int)diff_record.position, (long int)diff_record.length);
			printf("  DEBUG: Currently I am at position %ld\n", (long int)position);
			printf("  DEBUG: Copying %ld bytes from input file\n", (long int)(diff_record.position - position));
		}

		// Since records are sequentially ordered, we now copy everything up until position
		copy(file2, file1, diff_record.position - position, chunksize);
		position += diff_record.position - position;

		if(verbose) {
			printf("  DEBUG: Appending %ld bytes from patch file\n", (long int)diff_record.length);
		}

		// And then the patched sector
		copy(file2, patchfile, diff_record.length, chunksize);
		position += diff_record.length;
		fseek(file1, position, SEEK_SET); // Skip this in the input file

		if(verbose) {
			printf("  DEBUG: File positions:\n");
			printf("    FILE1 (input):  %ld\n", ftell(file1));
			printf("    FILE2 (output): %ld\n", ftell(file2));
			printf("    PATCH FILE:     %ld\n\n", ftell(patchfile));
		}
	}

	// Copy the rest of the sectors
	if(position != header.file2_size) {
		copy(file2, file1, header.file2_size - position, chunksize);
	}
	
	// gg ez
	return 0;
}

