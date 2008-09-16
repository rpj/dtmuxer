#include "dtM4AMuxer.h"

// Note how this array only contains the known parent atoms necessary
// to find and parse the 'meta' atom, which is what we're interested.
// This is by design.
static char* gKnownParentCodes[] = { "moov", "udta", NULL };

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
	uint32_t header[3];
	uint32_t size = sizeof(uint32_t) * 3;
	
	if (pread(filedes, header, size, 0) == size)
		return (!memcmp(FTYP_ATOM_ID, &header[1], sizeof(uint32_t)) && !memcmp(M4A_FILE_ID, &header[2], sizeof(uint32_t)));
	
	return NO;
}

void printMPEG4AtomToStdout(mpeg4atom_t* atom, const char* tabs)
{
	char code[5];
	memcpy(code, &atom->code, 4);
	code[4] = '\0';
	
	printf("%s'%s' <0x%x>:\tlength %d, data: 0x%x, parent: 0x%x, next: 0x%x, fChild: 0x%x\n",
		   tabs, code, atom, atom->length, atom->data, atom->parent, atom->next, atom->firstChild);
}

void printMPEG4StructureToStdout(mpeg4atom_t* m4a, const char* tabs)
{
	printMPEG4AtomToStdout(m4a, tabs);
	
	if (m4a->firstChild) {
		char* moreTabs = (char*)malloc(strlen(tabs) + 1 + 2);
		sprintf(moreTabs, "%s\t", tabs);
		printMPEG4StructureToStdout(m4a->firstChild, moreTabs);
	}
	
	if (m4a->next) printMPEG4StructureToStdout(m4a->next, tabs);
}