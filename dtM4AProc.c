#include "dtM4AMuxer.h"

BOOL removeAtomFromMPEG4(mpeg4atom_t* m4a, char* atomName)
{
	return NO;
}

void printMPEG4AtomToStdout(mpeg4atom_t* atom, const char* tabs)
{
	char code[5];
	memcpy(code, &atom->code, 4);
	code[4] = '\0';
	
	printf("%sAtom '%s' <0x%x>:\n\t%slength %d, data: 0x%x, parent: 0x%x, next: 0x%x, fChild: 0x%x\n\n",
		   tabs, code, atom, tabs, atom->length, atom->data, atom->parent, atom->next, atom->firstChild);
}

void printMPEG4StructureToStdout(mpeg4atom_t* m4a, const char* tabs)
{
	printMPEG4AtomToStdout(m4a, tabs);
	
	if (m4a->next) {
		printMPEG4StructureToStdout(m4a->next, tabs);
	}
	
	if (m4a->firstChild) {
		printf("\tChildren:\n");
		char* moreTabs = (char*)malloc(strlen(tabs) + 1 + 2);
		sprintf(moreTabs, "%s\t", tabs);
		printMPEG4StructureToStdout(m4a->firstChild, moreTabs);
	}
}