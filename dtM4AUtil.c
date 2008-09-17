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
	uint32_t header[3];
	uint32_t size = sizeof(uint32_t) * 3;
	
	if (pread(filedes, header, size, 0) == size)
		return (!memcmp(FTYP_ATOM_ID, &header[1], sizeof(uint32_t)) && !memcmp(M4A_FILE_ID, &header[2], sizeof(uint32_t)));
	
	return NO;
}

mpeg4atom_t* findAtomWithName(mpeg4atom_t* m4a, char* atomName)
{
	mpeg4atom_t* retAtom = NULL;
	printf("findWithName(%s)\n", atomName);
	
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

uint32_t peekAtNextAtomCode(mpeg4atom_t* curatom)
{
	if (curatom && curatom->data)
		return *((uint32_t*)((char*)curatom->data + curatom->length - sizeof(uint32_t)));
	
	return 0;
}

char* nameOfParentAtom(uint32_t code)
{
	printf("NAME OF got 0x%x\n", code);
	if (!memcmp("udta", &code, sizeof(uint32_t)))
		return "moov";
	
	return NULL;
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

void printMPEG4AtomDescriptionToStdout(mpeg4atom_t* atom, const char* tabs)
{
	char code[5];
	memcpy(code, &atom->code, 4);
	code[4] = '\0';
	
	printf("%s'%s' <0x%x>:\tlength %d, data: 0x%x, parent: 0x%x, next: 0x%x, fChild: 0x%x\n",
		   tabs, code, atom, atom->length, atom->data, atom->parent, atom->next, atom->firstChild);
}

void printMPEG4AtomToStdout(mpeg4atom_t* m4a, const char* tabs)
{
	printMPEG4AtomDescriptionToStdout(m4a, tabs);
	
	if (m4a->firstChild) {
		char* moreTabs = (char*)malloc(strlen(tabs) + 1 + 2);
		sprintf(moreTabs, "%s\t", tabs);
		printMPEG4AtomToStdout(m4a->firstChild, moreTabs);
	}
	
	if (m4a->next) printMPEG4AtomToStdout(m4a->next, tabs);
}

void printMPEG4FileToStdout(mpeg4file_t* file)
{
	printf("File length: %d\tdata: 0x%x\n", file->fileSize, file->fileData);
	printMPEG4AtomToStdout(file->rootAtom, "");
}
