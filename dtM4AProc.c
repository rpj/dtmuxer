#include "dtM4AMuxer.h"

mpeg4atom_t* findAtomWithName(mpeg4atom_t* m4a, char* atomName)
{
	mpeg4atom_t* retAtom = NULL;
	
	if (m4a) {
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

BOOL removeAtomFromMPEG4(mpeg4atom_t* m4a, char* atomName)
{
	mpeg4atom_t* atom = findAtomWithName(m4a, atomName);
	
	if (atom) {
		printf("atom is 0x%x; parent is 0x%x; grandparent is 0x%x\n", atom, (atom->parent ? atom->parent : NULL),
			   (atom->parent && atom->parent->parent ? atom->parent->parent : NULL));
	}
	
	return (atom != NULL);
}