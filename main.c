#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "diff.h"

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

int main(int argc, char *argv[]) {
    int c;

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
	    exit(128);
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

    return diff(file1, file2, outfile, chunksize, verboseopt);
}
