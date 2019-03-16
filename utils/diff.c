#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "diff.h"
#include "patch.h"
#include "format.h"

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

  // Create and initialize incomplete file header
  file_header_t header;
  header.magic = HEADER_MAGIC;
  header.version = HEADER_VERSION;
  header.incomplete = HEADER_INCOMPLETE_YES;
  header.chunk_size = chunksize;

  fseek(file1, 0, SEEK_END);
  header.file1_size = ftell(file1);
  fseek(file1, 0, SEEK_SET);

  fseek(file2, 0, SEEK_END);
  header.file2_size = ftell(file2);
  fseek(file2, 0, SEEK_SET);

  fwrite(&header, sizeof(file_header_t), 1, outfile);

  // Allocate buffers
  uint8_t *buffer1 = malloc(chunksize);
  uint8_t *buffer2 = malloc(chunksize);

  // Start diffing
  uint64_t position=0;
  while(1) {
    size_t bytecnt1 = fread(buffer1, 1, chunksize, file1);
    size_t bytecnt2 = fread(buffer2, 1, chunksize, file2);

    if(bytecnt1 != bytecnt2 || memcmp(buffer1, buffer2, chunksize) != 0) {
      if(verbose) {
        printf("  DEBUG: Found difference at position %ld\n", position);
      }
      // The files differ here. Write diff record.
      data_record_t diff_record;
      diff_record.position = position;
      diff_record.length = bytecnt2;

      if(verbose) {
        printf("  DEBUG: Diff record: Position: %ld, length: %ld\n", diff_record.position, diff_record.length);
      }
      fwrite(&diff_record.position, sizeof(diff_record.position), 1, outfile);
      fwrite(&diff_record.length,  sizeof(diff_record.length),  1, outfile);
      fwrite(buffer2, bytecnt2, 1, outfile);


	if(verbose) {
		printf("  DEBUG: File positions:\n");
		printf("    FILE1 (input):  %ld\n", ftell(file1));
		printf("    FILE2 (output): %ld\n", ftell(file2));
		printf("    PATCH FILE:     %ld\n\n", ftell(outfile));
	}
    }

    position += bytecnt2;

    // Check for file ends
    if(feof(file2)) {
      if(verbose) {
        printf("  DEBUG: File 2 ended. We are done.\n");
      }
      // We are done.
      break;
    }

    if(feof(file1)) {
      if(verbose) {
        printf("  DEBUG: File 1 ended. Appending rest of file2 as diff record.\n");
      }
      // file1 ended, just append the rest of file2
      data_record_t diff_record;
      diff_record.position = position;
      diff_record.length = header.file2_size - position;
      fwrite(&diff_record.position, sizeof(diff_record.position), 1, outfile);
      fwrite(&diff_record.length, sizeof(diff_record.length),  1, outfile);

      while(!feof(file2)) {
        bytecnt2 = fread(buffer2, 1, chunksize, file2);
        fwrite(buffer2, bytecnt2, 1, outfile);
      }

      break;
    }
  }

  // Write final header
  fseek(outfile, 0, SEEK_SET);
  header.incomplete = HEADER_INCOMPLETE_NO;
  fwrite(&header, sizeof(file_header_t), 1, outfile);

  return 0;
}

