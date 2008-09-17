#include "dtM4AMuxer.h"

BOOL mogrifyAtomIntoFreeSpace(mpeg4atom_t* m4a, char* atomName)
{
	mpeg4atom_t* atom = findAtomWithName(m4a, atomName);
	
	if (atom) {
		char* freeCode = "free";
		
		atom->code = *((uint32_t*)freeCode);
		bzero(atom->data, atom->length - (sizeof(uint32_t) * 2));
		
		return YES;
	}
	
	return NO;
}

BOOL removeAtomFromMPEG4ForReals(mpeg4atom_t* m4a, char* atomName)
{
	mpeg4atom_t* atom = findAtomWithName(m4a, atomName);
	
	if (atom) {
		mpeg4atom_t* parent = atom->parent;
		
		if (parent) {
			parent->firstChild = atom->next;
			
			// adjust atom lengths for the parent atom, it's parent (atom's grandparent), until no parent is found
			for (; parent; parent = parent->parent)
				parent->length -= atom->length;
		}
		
		if (atom->next) {
			// adjust the sibling chain here: will probably require that atoms have a 'previous' pointer too, dammit!
		}
		
		// must also adjust 'mdat' offset if it has moved (most likely!)
		
		// delete the memory for the atom, because it is no longer in the tree
		freeMPEG4Atom(atom);
	}
	
	return (atom != NULL);
}

BOOL removeAtomFromMPEG4(mpeg4file_t* m4afile, char* atomName)
{
	return mogrifyAtomIntoFreeSpace(m4afile->rootAtom, atomName);
	//return removeAtomFromMPEG4ForReals(m4afile->rootAtom, atomName);
}