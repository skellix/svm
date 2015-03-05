#ifndef BYTECODE_H
#define BYTECODE_H

#include <stdio.h>
#include <gc.h>

typedef struct _Bytecode{
	char* name;
	long length;
	char* data;
} Bytecode;

Bytecode* Bytecode_loadFromFile(char* fileName) {
	FILE* source = fopen(fileName, "rb");
	fseek(source, 0, SEEK_END);
	int length = ftell(source);
	fseek(source, 0, SEEK_SET);
	Bytecode* out = GC_MALLOC(sizeof(Bytecode));
	out->name = GC_MALLOC(sizeof(char*) * strlen(fileName));
	strcpy(out->name, fileName);
	out->length = length;
	out->data = GC_MALLOC(sizeof(char*) * length);
	fread(out->data, sizeof(char), length, source);
	fclose(source);
	return out;
}

#endif
