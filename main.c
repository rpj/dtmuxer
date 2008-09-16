#include "dtM4AMuxer.h"

int main (int argc, const char * argv[]) {
	int retVal = 0;
	
	if (argc > 1) {
		mpeg4atom_t* m4a = readMPEG4FileFromPath(argv[1]);
	}
	else {
		fprintf(stderr, "No MPEG-4 audio file given; exiting.\n\n");
		retVal = -1;
	}
	
    return retVal;
}
