// dtM4AWrite.c -- routines for writing MPEG-4 audio files and atoms to disk
// (C) 2008 Ryan Joseph

#include "dtM4AMuxer.h"

BOOL writeSingleAtom(mpeg4atom_t* m4a, int filedes)
{
	BOOL retVal = NO;
	uint32_t flopLength = htonl(m4a->length);
	
	// write the length first
	if (write(filedes, &flopLength, sizeof(uint32_t)) == sizeof(uint32_t)) {
		// then the code
		if (write(filedes, &m4a->code, sizeof(uint32_t)) == sizeof(uint32_t)) {
			// then the data itself
			uint32_t dataSize = m4a->length - (sizeof(uint32_t) * 2);
			
			if (write(filedes, m4a->data, dataSize) == dataSize)
				retVal = YES;
		}
	}
	
	return retVal;
}

BOOL writeAtomAndSiblingsToFile(mpeg4atom_t* m4a, int filedes)
{
	BOOL retVal = NO;
	
	if (filedes > 0) {
		if (m4a) {
			// If we're able to write this atom, try for it's siblings; DON'T write the children, 
			// because they are a subset of this top-level chain of sibling atoms and doing
			// so would create duplicate data. This has the side effect of requiring us to directly 
			// modify atom lengths in the mpeg4file_t structure, to avoid writing stale lengths.
			if ((retVal = writeSingleAtom(m4a, filedes)))
				retVal = writeAtomAndSiblingsToFile(m4a->next, filedes);
		}
		else
			retVal = YES;	// to ensure writeMPEG4FileToPath returns YES after recursion ends
	}
	
	return retVal;
}

BOOL writeMPEG4FileToPath(mpeg4file_t* m4afile, const char* path)
{
	BOOL retVal = NO;
	int fileDesc = open(path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	
	if (m4afile && path && fileDesc > 0) {
		retVal = writeAtomAndSiblingsToFile(m4afile->rootAtom, fileDesc);
		close(fileDesc);
	}
	
	return retVal;
}