#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

void help(char* name) {
    printf("Usage: %s [OPTION]... FILE1 FILE2 OUTPUTFILE\n", name);
    printf("Compary binary files chunk by chunk.\n");
    printf("\n");
    printf("-v, --verbose    Show verbose output\n");
    printf("-h, --help       Show usage information\n");
    printf("-s, --size=SIZE  Set chunk size to size (defaults to 4096)\n");
    printf("\n");
    exit(2);
}

int create_diff(char* file1name, char *file2name, char *outfilename, int chunksize, int verbose) {
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

int main(int argc, char *argv[]) {
    int c;
    int digit_optind = 0;
    static struct option long_options[] = {
        {"help", 0, NULL, 'h'},
        {"verbose", 0, NULL, 'v'},
	{"size", 1, NULL, 's'},
        {NULL, 0, NULL, 0}
    };
    int option_index = 0;
    int helpopt = 0;
    int verboseopt = 0;
    int chunksize = 4096;
    while ((c = getopt_long(argc, argv, "hv",
                 long_options, &option_index)) != -1) {
        switch (c) {
        case 'h':
	    helpopt = 1;
            break;
        case 'v':
	    verboseopt = 1;
            break;
	case 's':
	    chunksize = atoi(optarg);
	    break;
        case '?':
            break;
        default:
            printf ("?? getopt returned character code 0%o ??\n", c);
        }
    }

    char *file1 = NULL;
    char *file2 = NULL;
    char *outfile = NULL;

    if(helpopt) {
      help(argv[0]);
    }

    if (optind < argc) {
	file1 = argv[optind++];
    } else {
      help(argv[0]);
    }

    if (optind < argc) {
	file2 = argv[optind++];
    } else {
      help(argv[0]);
    }

    if (optind < argc) {
      outfile = argv[optind++];
    } else {
      help(argv[0]);
    }

    if (optind < argc) {
	    help(argv[0]);
    }

    if(verboseopt) {
	    printf("%s: DEBUG: Starting with chunk size %d\n", argv[0], chunksize);
    }

    if(chunksize < 1) {
      printf("%s: ERROR: chunk size %d is too small\n", argv[0], chunksize);
      help(argv[0]);
    }

    return create_diff(file1, file2, outfile, chunksize, verboseopt);
}
