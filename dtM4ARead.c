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
		printf("\nparse started with: 0x%x, %d, %d, 0x%x, 0x%x\n", data, dataLen, lastLen, parent, previous);
		atom = (mpeg4atom_t*)malloc(sizeof(mpeg4atom_t));
		
		atom->parent = parent;
		atom->length = ntohl(*(uint32_t*)data);
		atom->code = *(((uint32_t*)data) + 1);
		atom->data = (uint32_t*)data + 2;
		
		printMPEG4AtomToStdout(atom, "");
		
		if (previous && previous != parent) {
			printf("Setting previous[0x%x]->next = 0x%x\n", previous, atom);
			previous->next = atom;
		}
		
		void* nextDataPtr = ((char*)data) + atom->length;
		ssize_t newDataLen = dataLen - atom->length;
		
		// The call to atomCodeIsKnownParent() is here to provide a preset mapping of what atoms should be considered
		// parents. I've done things this way because of the fact that MPEG-4 audio files tend to break the "an atom
		// is either a parent OR a content atom" rule, making parsing the data structure without any foreknowledge
		// (such as this mapping) nearly impossible. Backtracking doesn't help, either, because the "if the current 
		// atom's size is less than the last" doesn't hold consistently.
		// Ensuring that the 'udta' atom is a known parent allows us to be sure that we will discover and parse
		// the 'meta' atom (since it is a direct child of 'udta'), which for the purposes of this program is "good enough."
		if (atomCodeIsKnownParent(atom->code)) {
			printf("parent atom: parse(0x%x, %d, %d, 0x%x, NULL)\n", atom->data, dataLen - (sizeof(uint32_t) * 2), atom->length - (sizeof(uint32_t) * 2), atom);
			atom->firstChild = parseMPEG4DataRec(atom->data, dataLen - (sizeof(uint32_t) * 2), atom->length - (sizeof(uint32_t) * 2), atom, atom);
		}
		else {
			mpeg4atom_t* newParent = parent;
			mpeg4atom_t* newBro = atom;
			uint32_t newLen = lastLen - atom->length;
			
			// if the length after this atom for this sub-tree is non-positive, then we've run out of data space
			// in this subtree and the next atom parsed will be a sibling of the curent atom's parent.
			if (newLen <= 0 && parent) {
				printf("Adjusting parent and siblings from 0x%x and 0x%x...", newParent, newBro);
				newBro = parent->parent;
				newParent = newBro ? newBro->parent : NULL;
				printf(" to 0x%x and 0x%x\n", newParent, newBro);
				newLen = atom->length;
			}
			
			printf("content atom: parse(0x%x, %d, %d, 0x%x, 0x%x)\n", nextDataPtr, newDataLen, newLen, newParent, newBro);
			mpeg4atom_t* new = parseMPEG4DataRec(nextDataPtr, newDataLen, lastLen, newParent, newBro);
			
			// if we haven't adjusted the sibling and parent pointers, the new node is the current atom's sibling
			if (newBro == atom && newParent == parent) {
				atom->next = new;
				printf("Set [0x%x]->next = 0x%x\n", atom, atom->next);
			}
		}
	}
	
	printf("parse returning with: 0x%x, %d, %d, 0x%x, 0x%x\n", data, dataLen, lastLen, parent, previous);
	return atom;
}

mpeg4atom_t* parseMPEG4Data(void* data, ssize_t length)
{
	return parseMPEG4DataRec(data, length, 0, NULL, NULL);
}

mpeg4atom_t* readMPEG4FileFromPath(const char* path)
{
	mpeg4atom_t* root = NULL;
	
	int filedes = open(path, O_RDONLY);
	
	if (filedes > 0) {
		char* fileData = NULL;
		ssize_t fileLength = 0;
		
		if (fileIsValidM4AFile(filedes)) {
			printf("Valid M4A; reading...\n");
			
			if ((fileLength = readFileIntoBuffer(filedes, (void**)&fileData))) {
				printf("starting parse(0x%x, %d)\n", fileData, (uint32_t)fileLength);
				root = parseMPEG4Data(fileData, fileLength);
				printf("got root atom 0x%x\n", root);
			}
		}
		else {
			fprintf(stderr, "Not a MPEG-4 audio file; exiting.\n");
		}
	}
	
	return root;
}