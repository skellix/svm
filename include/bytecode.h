#ifndef BYTECODE_H
#define BYTECODE_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "gc.h"
#include "error.h"
#include "opcodes.h"

typedef struct _Bytecode{
	char* name;
	long length;
	char* data;
	int lines;
	long* lineNumbers;
} Bytecode;

#include "stack.h"


void outputOps(Bytecode* main);

void debugOut(Bytecode* out, int* label, int labelLen, char* labelType, int i, int offset, int stringOffset);

Bytecode* Bytecode_loadFromFile(char* fileName, int debug);


#endif//BYTECODE_H
