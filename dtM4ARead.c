#include "dtM4AMuxer.h"

#define STATIC_BUFFER_SIZE		4096

static char gStaticBuffer[STATIC_BUFFER_SIZE];

static char* gKnownParentCodes[] = { "moov", "udta", NULL };

BOOL atomCodeIsKnownParent(uint32_t code) 
{
	BOOL retVal = NO;
	char* cPtr = *gKnownParentCodes;
	uint32_t count = 1;
	
	for (; !retVal && cPtr; cPtr = *(gKnownParentCodes + count++))
		retVal = !memcmp((void*)cPtr, &code, sizeof(uint32_t));
	
	return retVal;
}

mpeg4atom_t* lastChildOf(mpeg4atom_t* atom)
{
	printf("\nlastChildOf\n"); printMPEG4AtomToStdout(atom, "\t\t"); printf("\n\n");
	mpeg4atom_t* kid = atom->firstChild;
	
	printf("lastChildOf 0x%x, starting with 0x%x\n", atom, kid);
	for(; kid && kid->next; kid = kid->next) {
		printf("0x%x not last, moving on\n", kid);
	}
	
	return kid;
}

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
		
		char code[5];
		memcpy(code, &atom->code, 4);
		code[4] = '\0';
		printf("new atom 0x%x: p 0x%x, l 0x%x, c '%s', d 0x%x\n", atom, atom->parent, atom->length, code, atom->data);
		
		if (previous && previous != parent) {
			printf("Setting previous[0x%x]->next = 0x%x\n", previous, atom);
			previous->next = atom;
		}
		
		void* nextDataPtr = ((char*)data) + atom->length;
		ssize_t newDataLen = dataLen - atom->length;
		
		if (atomCodeIsKnownParent(atom->code)) {
			printf("parent atom: parse(0x%x, %d, %d, 0x%x, NULL)\n", atom->data, dataLen - (sizeof(uint32_t) * 2), atom->length - (sizeof(uint32_t) * 2), atom);
			atom->firstChild = parseMPEG4DataRec(atom->data, dataLen - (sizeof(uint32_t) * 2), atom->length - (sizeof(uint32_t) * 2), atom, atom);
		}
		else {
			mpeg4atom_t* newParent = parent;
			mpeg4atom_t* newBro = atom;
			uint32_t newLen = lastLen - atom->length;
			
			if (newLen <= 0 && parent) {
				printf("Adjusting parent and siblings from 0x%x and 0x%x...", newParent, newBro);
				newBro = parent->parent;
				newParent = newBro ? newBro->parent : NULL;
				printf(" to 0x%x and 0x%x\n", newParent, newBro);
				newLen = atom->length;
			}
			
			printf("content atom: parse(0x%x, %d, %d, 0x%x, 0x%x)\n", nextDataPtr, newDataLen, lastLen, newParent, newBro);
			atom->next = parseMPEG4DataRec(nextDataPtr, newDataLen, lastLen, newParent, newBro);
		}
			
		/*
		uint32_t nextAtomLength = 0;
		
		if (atom->length < dataLen) {
			nextAtomLength = ntohl(*(uint32_t*)(((char*)data) + atom->length));
			printf("Next atom length is 0x%x\n", nextAtomLength);
		}
		
		if (lastLen < 3) {
			if (nextAtomLength < atom->length) {
				printf("parent atom: parse(0x%x, %d, %d, 0x%x, NULL)\n", atom->data, atom->length, lastLen + 1, atom);
				atom->firstChild = parseMPEG4DataRec(atom->data, atom->length, lastLen + 1, atom, NULL);
			} else {
				
				//if (nextAtomLength > atom->length) {
				printf("content atom: parse(0x%x, %d, %d, 0x%x, 0x%x)\n", nextDataPtr, newDataLen, lastLen, parent, atom);
				atom->next = parseMPEG4DataRec(nextDataPtr, newDataLen, lastLen, parent, atom);
			}
		}

		
		 
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
		 */
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