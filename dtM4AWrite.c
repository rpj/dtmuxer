#include "dtM4AMuxer.h"

BOOL writeSingleAtom(mpeg4atom_t* m4a, int filedes)
{
	uint32_t flopLength = htonl(m4a->length);
	
	// write the length first
	if (write(filedes, &flopLength, sizeof(uint32_t)) == sizeof(uint32_t)) {
		// then the code
		if (write(filedes, &m4a->code, sizeof(uint32_t)) == sizeof(uint32_t)) {
			// then the data itself
			uint32_t dataSize = m4a->length - (sizeof(uint32_t) * 2);
			
			if (write(filedes, m4a->data, dataSize) == dataSize)
				return YES;
		}
	}
	
	return NO;
}

BOOL writeAtomAndSiblingsToFile(mpeg4atom_t* m4a, int filedes)
{
	BOOL retVal = NO;
	
	if (m4a && filedes > 0) {
		// if we're able to write this atom, try for it's siblings; DON'T write the children, 
		// because they are a subset of this top-level chain of sibling atoms and doing
		// so would create duplicate data.
		if ((retVal = writeSingleAtom(m4a, filedes)))
			retVal = writeAtomAndSiblingsToFile(m4a->next, filedes);
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