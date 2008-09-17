// dtM4AMuxer.h -- defintions for dtM4AMuxer project
// (C) 2008 Ryan Joseph

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define YES (1)
#define NO	(0)

#define FTYP_ATOM_ID			"ftyp"
#define M4A_FILE_ID				"M4A "

typedef char BOOL;

typedef struct mpeg4atom {
	uint32_t				length;
	uint32_t				code;
	void*					data;
	struct mpeg4atom*		parent;
	struct mpeg4atom*		next;
	struct mpeg4atom*		firstChild;
} mpeg4atom_t;

typedef struct mpeg4file {
	mpeg4atom_t*			rootAtom;
	ssize_t					fileSize;
	void*					fileData;
} mpeg4file_t;

// dtM4ARead.c
mpeg4file_t* readMPEG4FileFromPath(const char*);

// dtM4AWrite.c
BOOL writeMPEG4FileToPath(mpeg4file_t*, const char*);

// dtM4AProc.c
BOOL removeAtomFromMPEG4(mpeg4file_t*, char*);

// dtM4AUtil.c
BOOL fileIsValidM4AFile(int);
BOOL atomCodeIsKnownParent(uint32_t);

mpeg4atom_t* findAtomWithName(mpeg4atom_t*, char*);
char* nameOfParentAtom(uint32_t code);
uint32_t peekAtNextAtomCode(mpeg4atom_t* curatom);

void freeMPEG4Atom(mpeg4atom_t*);
void freeMPEG4File(mpeg4file_t* m4afile);

void printMPEG4AtomToStdout(mpeg4atom_t*, const char*);
void printMPEG4StructureToStdout(mpeg4atom_t*, const char*);
void printMPEG4FileToStdout(mpeg4file_t* file);