#include "dtM4AMuxer.h"

int main (int argc, const char * argv[]) {
	int retVal = 0;
	
	if (argc > 1) {
		mpeg4file_t* m4a = readMPEG4FileFromPath(argv[1]);
		
		
		if (m4a) { 
			printf("\nMPEG4 structure:\n\n");
			printMPEG4FileToStdout(m4a);
			printf("\n\n");
			
			printf("Writing pristine:\n");
			writeMPEG4FileToPath(m4a, "OUT-PRISTINE.m4a");
			
			if (removeAtomFromMPEG4(m4a, "meta")) {				
				printf("\nMPEG4 structure after remove:\n\n");
				printMPEG4FileToStdout(m4a);
				printf("\n\n");
				
				printf("Writing.\n");
				writeMPEG4FileToPath(m4a, "OUT.m4a");
			}
			else {
				printf("No 'meta' atom found, nothing to do; exiting.\n\n");
			}
			
			printf("FREEING\n");
			freeMPEG4File(m4a);
		}
	}
	else {
		fprintf(stderr, "No MPEG-4 audio file given; exiting.\n\n");
		retVal = -1;
	}
	
    return retVal;
}
