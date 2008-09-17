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
	
	if (data && dataLen) {
		atom = (mpeg4atom_t*)malloc(sizeof(mpeg4atom_t));
		
		atom->parent = parent;
		atom->length = ntohl(*(uint32_t*)data);
		atom->code = *(((uint32_t*)data) + 1);
		atom->data = (uint32_t*)data + 2;
		
		// This algorithm for deciding parent/child relationships is a bit off; it doesn't affect the 
		// overall functionality of the program because the tree gets built correctly "enough" to be valid
		// when rewritten, but if I'd had more time I believe this function - and this code specifically -
		// would be where I'd spend a lot more time. See note (1) later in this function for the other
		// corresponding code that is involved in the parent/child determination.
		mpeg4atom_t* temp = parent;
		for (; temp && temp->parent; temp = temp->parent);
		mpeg4atom_t* aNewDad = findAtomWithName(temp, nameOfParentAtom(atom->code));
		
		if (aNewDad) {
			atom->parent = aNewDad;
			
			if (aNewDad->firstChild)
				aNewDad->firstChild->next = atom;
		}
		
		// setup the sibling chain, if given a previous atom that is not the parent atom
		if (previous && previous != parent)
			previous->next = atom;
		
		// The call to atomCodeIsKnownParent() is here to provide a preset mapping of what atoms should be considered
		// parents. I've done things this way because of the fact that MPEG-4 audio files tend to break the "an atom
		// is either a parent OR a content atom" rule, making parsing the data structure without any foreknowledge
		// (such as this mapping) nearly impossible. Backtracking doesn't help, either, because the "if the current 
		// atom's size is less than the last then it is a parent atom" doesn't hold consistently.
		if (atomCodeIsKnownParent(atom->code)) {
			uint32_t adjSize = sizeof(uint32_t) * 2;
			
			// since we pass in (atom->length - adjSize) here for the lastLen parameter, we are limiting the
			// children of this atom to only the data given for this atom; as it should be.
			atom->firstChild = parseMPEG4DataRec(atom->data, dataLen - adjSize, atom->length - adjSize, atom, atom);
		}
		else {
			void* newDataPtr = ((char*)data) + atom->length;
			ssize_t newDataLen = dataLen - atom->length;
			mpeg4atom_t* trueParent = parent;
			mpeg4atom_t* trueSibling = atom;
			
			// (1); see first comment in this function.
			if ((lastLen - atom->length) <= 0 && parent) {
				trueSibling = parent->parent;
				trueParent = trueSibling ? trueSibling->parent : NULL;
			}
			
			mpeg4atom_t* new = parseMPEG4DataRec(newDataPtr, newDataLen, lastLen - atom->length, trueParent, trueSibling);
			
			// if we haven't adjusted the sibling and parent pointers, the new node is the current atom's sibling
			if (trueSibling == atom && trueParent == parent)
				atom->next = new;
		}
	}
	
	return atom;
}

// NOTE: free()s the data passed in... but that's what you wanted to do, right?
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