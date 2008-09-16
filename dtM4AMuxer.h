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

// dtM4ARead.c
mpeg4atom_t* readMPEG4FileFromPath(const char*);

// dtM4AWrite.c
BOOL writeMPEG4FileToPath(mpeg4atom_t*, const char*);

// dtM4AProc.c
BOOL removeAtomFromMPEG4(mpeg4atom_t*, char*);

// dtM4AUtil.c
BOOL fileIsValidM4AFile(int filedes);
BOOL atomCodeIsKnownParent(uint32_t code);

void printMPEG4AtomToStdout(mpeg4atom_t* atom, const char* tabs);
void printMPEG4StructureToStdout(mpeg4atom_t* m4a, const char*);