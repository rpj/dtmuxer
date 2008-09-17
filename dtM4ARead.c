// dtM4ARead.c -- routines for reading and parsing MPEG-4 audio data
// (C) 2008 Ryan Joseph

#include "dtM4AMuxer.h"

#define STATIC_BUFFER_SIZE		4096

static char gStaticBuffer[STATIC_BUFFER_SIZE];

ssize_t readFileIntoBuffer(int filedes, void** buffer)
{
	ssize_t lastReadLen = -1;
	ssize_t retSize = 0;
	char* buildBuffer = NULL;
	
	// read chunks of the file (hopefully disk block-sized chunks) into a static buffer, and build up the full
	// return buffer from these chunks
	for (; (lastReadLen = read(filedes, (void*)gStaticBuffer, STATIC_BUFFER_SIZE)) > 0; retSize += lastReadLen) {
		buildBuffer = (char*)(buildBuffer ? realloc(buildBuffer, lastReadLen + retSize) : malloc(lastReadLen));
		memcpy((buildBuffer + retSize), gStaticBuffer, lastReadLen);
	}
	
	if (buildBuffer)
		*buffer = (void*)buildBuffer;
	
	return retSize;
}

mpeg4atom_t* parseMPEG4DataRec(void* data, ssize_t dataLen, ssize_t lastLen, mpeg4atom_t* parent, mpeg4atom_t* previous)
{
	mpeg4atom_t* atom = NULL;
	printf("\n-----PARSE START-----\n");
	
	if (data && dataLen) {
		atom = (mpeg4atom_t*)malloc(sizeof(mpeg4atom_t));
		
		atom->parent = parent;
		atom->length = ntohl(*(uint32_t*)data);
		atom->code = *(((uint32_t*)data) + 1);
		atom->data = (uint32_t*)data + 2;
		
		
		mpeg4atom_t* temp = parent;
		for (; temp && temp->parent; temp = temp->parent);
		mpeg4atom_t* aNewDad = findAtomWithName(temp, nameOfParentAtom(atom->code));
		if (aNewDad) printf("aNewDad: %x\n", aNewDad);
		
		// setup the sibling chain, if given a previous atom that is not the parent atom
		if (previous && previous != parent)
			previous->next = atom;
		
		if (aNewDad) {
			printf("Getting a new daddy!\n");
			atom->parent = aNewDad;
			
			if (aNewDad->firstChild)
				aNewDad->firstChild->next = atom;
		}
		
		
		printMPEG4AtomToStdout(atom, "\n>>>>>>>> ");
		printf("datalen: %d (%x)\tlastlen: %d (%x)\n", dataLen, dataLen, lastLen, lastLen);
		printf("parent: %x\tprevious: %x\n", parent, previous);
		
		mpeg4atom_t* newParent	= parent;
		mpeg4atom_t* newBro		= atom;
		
		// If the length of data after this atom for this sub-tree is non-positive, then we've run out of data space
		// in this subtree and the next atom parsed will be a sibling of the curent atom's parent.
		/*if ((lastLen - atom->length) <= 0 && parent) {
			mpeg4atom_t* par = parent;
			for (; par && par->parent; par = par->parent);
			
			mpeg4atom_t* nPar = NULL;
			if ((nPar = findAtomWithName(par, nameOfParentAtom(peekAtNextAtomCode(atom))))) {
				printf("Found parent for current atom: ");
				printMPEG4AtomToStdout(nPar, "---->>>> ");
				
				newBro = nPar->firstChild;
				newParent = nPar;
			}
			else {
				newBro = par;
				newParent = newBro ? newBro->parent : NULL;
			}
			
			printf("parent: %x, newParent: %x, newBro: %x\n", parent, newParent, newBro);
		}*/
		
		// The call to atomCodeIsKnownParent() is here to provide a preset mapping of what atoms should be considered
		// parents. I've done things this way because of the fact that MPEG-4 audio files tend to break the "an atom
		// is either a parent OR a content atom" rule, making parsing the data structure without any foreknowledge
		// (such as this mapping) nearly impossible. Backtracking doesn't help, either, because the "if the current 
		// atom's size is less than the last then it is a parent atom" doesn't hold consistently.
		// Ensuring that the 'udta' atom is a known parent allows us to be sure that we will discover and parse
		// the 'meta' atom (since it is a direct child of 'udta') which, for the purposes of this program, will do.
		if (atomCodeIsKnownParent(atom->code)) { printf("PARENT\n");
			uint32_t adjSize = sizeof(uint32_t) * 2;
			
			newBro = newParent = atom;
			
			// since we pass in (atom->length - adjSize) here for the lastLen parameter, we are limiting the
			// children of this atom to only the data given for this atom; as it should be.
			atom->firstChild = parseMPEG4DataRec(atom->data, dataLen - adjSize, atom->length - adjSize, newParent, newBro);
		}
		else { printf("CHILD\n");
			void* newDataPtr		= ((char*)data) + atom->length;
			ssize_t newDataLen		= dataLen - atom->length;
			
			if ((lastLen - atom->length) <= 0 && parent) {
				newBro = parent->parent;
				newParent = newBro ? newBro->parent : NULL;
			}
			
			mpeg4atom_t* new = parseMPEG4DataRec(newDataPtr, newDataLen, lastLen - atom->length, newParent, newBro);
			
			// if we haven't adjusted the sibling and parent pointers, the new node is the current atom's sibling
			if (newBro == atom && newParent == parent)
				atom->next = new;
		}
	}
	
	return atom;
}

// NOTE: frees the data passed in!
mpeg4file_t* parseMPEG4Data(void* data, ssize_t length)
{
	mpeg4file_t* newFile = NULL;
	
	if (data && length) {
		newFile = (mpeg4file_t*)malloc(sizeof(struct mpeg4file));
	
		newFile->fileData = malloc(length);
		
		if (newFile->fileData) {
			newFile->fileSize = length;
			
			memcpy(newFile->fileData, data, length);
			free(data);
			
			newFile->rootAtom = parseMPEG4DataRec(newFile->fileData, length, 0, NULL, NULL);
		}
	}
	
	return newFile;
}

mpeg4file_t* readMPEG4FileFromPath(const char* path)
{
	mpeg4file_t* root = NULL;
	
	int filedes = open(path, O_RDONLY);
	
	if (filedes > 0) {
		char* fileData = NULL;
		ssize_t fileLength = 0;
		
		if (fileIsValidM4AFile(filedes)) {
			printf("Valid M4A; reading...\n");
			
			if ((fileLength = readFileIntoBuffer(filedes, (void**)&fileData)))
				root = parseMPEG4Data(fileData, fileLength);
		}
		else {
			fprintf(stderr, "Not a MPEG-4 audio file; exiting.\n");
		}
	}
	
	return root;
}