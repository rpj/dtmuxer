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

BOOL dataHasValidM4AHeader(char* data)
{
	uint32_t* intPtr = ((uint32_t*)data) + 1;	// skip the first 4b; they are the atom's length
	return (!memcmp("ftyp", intPtr, sizeof(uint32_t)) && !memcmp("M4A ", (intPtr + 1), sizeof(uint32_t)));
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
		
		if (previous) previous->next = atom;
		
		printf("new atom 0x%x: p 0x%x, l 0x%x, c 0x%x, d 0x%x\n", atom, atom->parent, atom->length, atom->code, atom->data);
		
		void* nextDataPtr = ((char*)data) + atom->length;
		ssize_t newDataLen = dataLen - atom->length;
		
		if (atom->length > lastLen) {
			// first atom, ftyp, is a special case
			if (!lastLen && !memcmp("ftyp", (void*)&atom->code, sizeof(uint32_t))) {
				printf("ftyp, first atom: parse(0x%x, %d, %d, 0x%x, 0x%x)\n", nextDataPtr, newDataLen, atom->length, NULL, atom);
				atom->next = parseMPEG4DataRec(nextDataPtr, newDataLen, atom->length, NULL, atom);
			}
			else {
				// this atom is a new parent atom
				printf("new parent: parse(0x%x, %d, %d, 0x%x, NULL)\n", atom->data, dataLen - (sizeof(uint32_t) * 2), atom->length, atom);
				atom->firstChild = parseMPEG4DataRec(atom->data, dataLen - (sizeof(uint32_t) * 2), atom->length, atom, NULL);
			}
		}
		else {
			// this is a child, or content, atom
			// passing 'parent' here may not be necessary
			printf("content atom: parse(0x%x, %d, %d, 0x%x, 0x%x)\n", nextDataPtr, newDataLen, atom->length, parent, atom);
			atom->next = parseMPEG4DataRec(nextDataPtr, newDataLen, atom->length, parent, atom);
		}
	}
	
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
		
		if ((fileLength = readFileIntoBuffer(filedes, (void**)&fileData))) {
			if (dataHasValidM4AHeader(fileData)) {
				printf("starting parse(0x%x, %d)\n", fileData, (uint32_t)fileLength);
				root = parseMPEG4Data(fileData, fileLength);
				printf("got root atom 0x%x\n", root);
			}
		}
	}
	
	return root;
}