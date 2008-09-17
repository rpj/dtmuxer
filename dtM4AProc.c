// dtM4AProc.c -- routines for processing and modifying MPEG-4 data and structures
// (C) 2008 Ryan Joseph

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
	mpeg4atom_t* stco = findAtomWithName(m4a, "stco");
	
	if (atom && stco) {
		mpeg4atom_t* parent = atom->parent;
		
		if (parent) {
			parent->firstChild = atom->next;
			
			// adjust atom lengths for the parent atom, it's parent (atom's grandparent), until no parent is found
			for (; parent; parent = parent->parent) {
				printf("Adjusting length for atom 0x%x by -%d\n", parent, atom->length);
				parent->length -= atom->length;
			}
		}
		
		if (atom->next) {
			// adjust the sibling chain here: will probably require that atoms have a 'previous' pointer too, dammit!
		}
		
		// here we adjust the chunk offsets for the media data, given in the 'stco' atom
		uint32_t* stcoIPtr = (uint32_t*)stco->data;
		uint32_t numOffsets = ntohl(*(stcoIPtr + 1));		// first byte of stco is ver num and flags; ignore
		uint32_t* curPtr = stcoIPtr + 2;
		uint32_t atomLen = atom->length;
		
		for (; numOffsets; numOffsets--, curPtr++)
			*curPtr = htonl(ntohl(*curPtr) - atomLen);
		
		// delete the memory for the atom, because it is no longer in the tree
		freeMPEG4Atom(atom);
	}
	
	return (atom != NULL);
}

BOOL removeAtomFromMPEG4(mpeg4file_t* m4afile, char* atomName)
{
	//return mogrifyAtomIntoFreeSpace(m4afile->rootAtom, atomName);
	return removeAtomFromMPEG4ForReals(m4afile->rootAtom, atomName);
}