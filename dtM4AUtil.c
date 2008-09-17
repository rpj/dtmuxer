// dtM4AUtil.c -- utility routines for dealing with MPEG-4 data and structures
// (C) 2008 Ryan Joseph

#include "dtM4AMuxer.h"

// Note how this array only contains the known parent atoms necessary
// to find and parse the 'meta' atom, which is what we're interested.
// This is by design.
static char* gKnownParentCodes[] = { "moov", "udta", "trak", "mdia", "minf", "stbl", NULL };

BOOL atomCodeIsKnownParent(uint32_t code) 
{
	BOOL retVal = NO;
	char* cPtr = *gKnownParentCodes;
	uint32_t count = 1;
	
	for (; !retVal && cPtr; cPtr = *(gKnownParentCodes + count++))
		retVal = !memcmp((void*)cPtr, &code, sizeof(uint32_t));
	
	return retVal;
}

BOOL fileIsValidM4AFile(int filedes)
{
	BOOL retVal = NO;
	uint32_t header[3];
	uint32_t size = sizeof(uint32_t) * 3;
	
	if (pread(filedes, header, size, 0) == size)
		retVal = (!memcmp(FTYP_ATOM_ID, &header[1], sizeof(uint32_t)) && !memcmp(M4A_FILE_ID, &header[2], sizeof(uint32_t)));
	
	return retVal;
}

mpeg4atom_t* findAtomWithName(mpeg4atom_t* m4a, char* atomName)
{
	mpeg4atom_t* retAtom = NULL;
	
	if (m4a && atomName) {
		if (!memcmp((uint32_t*)atomName, &m4a->code, sizeof(uint32_t)))
			retAtom = m4a;
		else {
			if (m4a->firstChild)
				retAtom = findAtomWithName(m4a->firstChild, atomName);
			
			if (!retAtom && m4a->next)
				retAtom = findAtomWithName(m4a->next, atomName);
		}
	}
	
	return retAtom;
}

// This routine would be extended to map all parent/child atom relationships
// if the project spec req'd us to deal with all the atoms, or if I had a lot more time.
char* nameOfParentAtom(uint32_t code)
{
	char* retVal = NULL;
	
	if (!memcmp("udta", &code, sizeof(uint32_t)))
		retVal = "moov";
	
	return retVal;
}

void freeMPEG4Atom(mpeg4atom_t* m4a)
{
	if (m4a->firstChild) freeMPEG4Atom(m4a->firstChild);
	
	mpeg4atom_t* next = m4a->next;
	free(m4a);
	
	if (next) freeMPEG4Atom(next);
}

void freeMPEG4File(mpeg4file_t* m4afile)
{
	freeMPEG4Atom(m4afile->rootAtom);
	free(m4afile->fileData);
}