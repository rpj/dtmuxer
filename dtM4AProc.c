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
		mpeg4atom_t* parent = atom->parent;
		
		if (parent) {
			parent->firstChild = atom->next;
			
			for (; parent; parent = parent->parent) {
				printf("Need to adjust length of 0x%x from %d to %d\n", parent, parent->length, parent->length - atom->length);
				parent->length -= atom->length;
			}
		}
		
		if (atom->next) {
			// adjust the sibling chain here: will probably require that atoms have a 'previous' pointer too, dammit!
		}
		
		// must also adjust 'mdat' offset if it has moved (most likely!)
		
		// delete the memory for the atom, because it is no longer in the tree
		free(atom);
	}
	
	return (atom != NULL);
}