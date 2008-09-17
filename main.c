// main.c -- entry point for dtM4AMuxer project
// (C) 2008 Ryan Joseph

#include "dtM4AMuxer.h"

int main (int argc, const char * argv[]) {
	int retVal = 0;
	
	if (argc > 1) {
		const char* fName = argv[1];
		mpeg4file_t* m4a = readMPEG4FileFromPath(fName);
		
		
		if (m4a) {
			if (removeAtomFromMPEG4(m4a, "meta")) {
				char *outFname = (char*)malloc(strlen(fName) + strlen(OUTPUT_EXTENSION) + 1);
				sprintf(outFname, "%s%s", fName, OUTPUT_EXTENSION);
				
				if (writeMPEG4FileToPath(m4a, outFname))
					printf("Removed 'meta' atom and wrote '%s' successfully.\n", outFname);
				else
					fprintf(stderr, "Unable to write '%s'; exiting.\n", outFname);
			}
			else {
				printf("No 'meta' atom found, nothing to do; exiting.\n");
			}
			
			freeMPEG4File(m4a);
		}
		else {
			fprintf(stderr, "'%s' is not a valid MPEG-4 audio file; exiting.\n", fName);
		}
	}
	else {
		fprintf(stderr, "Usage: %s [infile]\n\nIf successful, will produce a new file named '[infile]%s'\n", 
				argv[0], OUTPUT_EXTENSION);
		retVal = -1;
	}
	
    return retVal;
}
