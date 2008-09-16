#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define YES (1)
#define NO	(0)

typedef char BOOL;

typedef struct mpeg4atom {
	uint32_t				length;
	void*					data;
	struct mpeg4atom*		parent;
	struct mpeg4atom*		firstChild;
} mpeg4atom_t;

typedef struct mpeg4file {
} mpeg4file_t;

// dtM4ARead.c
BOOL dataHasValidM4AHeader(char* data);
ssize_t readFileIntoBuffer(int filedes, void** buffer);

mpeg4file_t* readMPEG4FileFromPath(const char* path);

// dtM4AWrite.c
// BOOL writeMPEG4ToPath(mpeg4file_t* mfile, char* path);

// dtM4AProc.c
// BOOL removeAtomNamed(char* name);

// util?
// void freeMPEG4File(mpeg4file_t* fileToFree); ??