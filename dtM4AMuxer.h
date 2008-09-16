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
	uint32_t				code;
	void*					data;
	struct mpeg4atom*		parent;
	struct mpeg4atom*		next;
	struct mpeg4atom*		firstChild;
} mpeg4atom_t;

// dtM4ARead.c
BOOL dataHasValidM4AHeader(char*);
ssize_t readFileIntoBuffer(int, void**);

mpeg4atom_t* readMPEG4FileFromPath(const char*);

// dtM4AWrite.c
BOOL writeMPEG4FileToPath(mpeg4atom_t*, const char*);

// dtM4AProc.c
BOOL removeAtomFromMPEG4(mpeg4atom_t*, char*);

void printMPEG4StructureToStdout(mpeg4atom_t* m4a, const char*);

// util?
// void freeMPEG4File(mpeg4file_t* fileToFree); ??