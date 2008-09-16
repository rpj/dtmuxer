#include "dtM4AMuxer.h"

#define STATIC_BUFFER_SIZE		4096
static char gStaticBuffer[STATIC_BUFFER_SIZE];

ssize_t readFileIntoBuffer(int filedes, void** buffer)
{
	ssize_t lastReadLen = -1;
	ssize_t retSize = 0;
	char* buildBuffer = NULL;
	
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

mpeg4file_t* readMPEG4FileFromPath(const char* path)
{
	mpeg4file_t* root = NULL;
	
	int filedes = open(path, O_RDONLY);
	
	if (filedes > 0) {
		char* fileData = NULL;
		ssize_t fileLength = 0;
		
		if ((fileLength = readFileIntoBuffer(filedes, (void**)&fileData))) {
			if (dataHasValidM4AHeader(fileData)) {
				// looks like we have m4a, build the data struct...
			}
		}
	}
	
	return root;
}