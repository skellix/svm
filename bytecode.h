#ifndef BYTECODE_H
#define BYTECODE_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <gc.h>

typedef struct _Bytecode{
	char* name;
	long length;
	short* data;
} Bytecode;

Bytecode* Bytecode_loadFromFile(char* fileName) {
	//struct timeval timeStart, timeEnd;
	//gettimeofday(&timeStart, NULL);
	FILE* source = fopen(fileName, "rb");
	if (source == NULL) {
		printf("Error: The source file could not be read!!!\n");
		exit(1);
	}
	fseek(source, 0, SEEK_END);
	int length = ftell(source);
	fseek(source, 0, SEEK_SET);
	Bytecode* out = GC_MALLOC(sizeof(Bytecode));
	out->name = GC_MALLOC(sizeof(char) * strlen(fileName));
	strcpy(out->name, fileName);
	out->length = length;
	out->data = GC_MALLOC(sizeof(short) * length);

	int c = -1;
	int labelLen = 0;
	int* label = GC_MALLOC(sizeof(int));
	char* labelType = GC_MALLOC(sizeof(char));
	int delimiter = 0;
	int offset = 0;
	int inString = 0;
	int stringOffset = 0;
	for (int i = 0 ; i < length ; i ++) {
		c = (char) fgetc(source);
		if (inString) {
			if (delimiter) {
				delimiter = 0;
				if (c == '\\') {
					*(((char*)&out->data[(i-offset)-stringOffset]) + stringOffset) = '\\';
				} else if (c == 'n') {
					*(((char*)&out->data[(i-offset)-stringOffset]) + stringOffset) = '\n';
				} else if (c == 't') {
					*(((char*)&out->data[(i-offset)-stringOffset]) + stringOffset) = '\t';
				} else if (c == 'r') {
					*(((char*)&out->data[(i-offset)-stringOffset]) + stringOffset) = '\r';
				} else {
					printf("character '%c' does not need to be delimited!!!\n");
				}
			} else {
				if (c == '\\') {
					delimiter = 1;
					stringOffset --;
					offset ++;
				} else if (c == '"') {
					inString = 0;
					*(((char*)&out->data[(i-offset)-stringOffset]) + stringOffset) = (char) 0;
					int start = label[labelLen - 1];
					out->data[start] = ((i-offset)-start) | (1 << 7);
					label = GC_REALLOC(label, sizeof(int) * (-- labelLen));
					labelType = GC_REALLOC(labelType, sizeof(char) * labelLen);
				} else if (c == '(') {
					inString = 0;
					*(((char*)&out->data[(i-offset)-stringOffset]) + stringOffset) = (char) 0;
					int start = label[labelLen - 1];
					out->data[start] = ((i-offset)-start) | (1 << 8);
					label = GC_REALLOC(label, sizeof(int) * (-- labelLen));
					labelType = GC_REALLOC(labelType, sizeof(char) * labelLen);
				} else if (c == '$') {
					inString = 0;
					*(((char*)&out->data[(i-offset)-stringOffset]) + stringOffset) = (char) 0;
					int start = label[labelLen - 1];
					out->data[start] = ((i-offset)-start) | (1 << 9);
					label = GC_REALLOC(label, sizeof(int) * (-- labelLen));
					labelType = GC_REALLOC(labelType, sizeof(char) * labelLen);
				} else if (c == '@') {
					inString = 0;
					*(((char*)&out->data[(i-offset)-stringOffset]) + stringOffset) = (char) 0;
					int start = label[labelLen - 1];
					out->data[start] = ((i-offset)-start) | (1 << 10);
					label = GC_REALLOC(label, sizeof(int) * (-- labelLen));
					labelType = GC_REALLOC(labelType, sizeof(char) * labelLen);
				} else {
					*(((char*)&out->data[(i-offset)-stringOffset]) + stringOffset) = (char) c;
				}
			}
			stringOffset ++;
			/*
			for (char* j = (char*)&out->data[label[labelLen - 1]] ; j < &out->data[i-offset] ; j ++) {
				printf("stringHas: %4d at: %d\n", (int) *j, j);
			}
			printf("stringPos: %d sdata: %c ", (((char*)&out->data[(i-offset)-stringOffset]) + stringOffset), *(((char*)&out->data[(i-offset)-stringOffset]) + stringOffset));
			//*/
		} else {
			if (c == '"') {
				inString = 1;
				stringOffset = 0;
				label = GC_REALLOC(label, sizeof(int) * (++ labelLen));
				labelType = GC_REALLOC(labelType, sizeof(char) * labelLen);
				label[labelLen - 1] = i - offset;
				labelType[labelLen - 1] = c;
			} else if (c == '!') {
				inString = 1;
				stringOffset = 0;
				label = GC_REALLOC(label, sizeof(int) * (++ labelLen));
				labelType = GC_REALLOC(labelType, sizeof(char) * labelLen);
				label[labelLen - 1] = i - offset;
				labelType[labelLen - 1] = c;
			} else if (c == '$') {
				inString = 1;
				stringOffset = 0;
				label = GC_REALLOC(label, sizeof(int) * (++ labelLen));
				labelType = GC_REALLOC(labelType, sizeof(char) * labelLen);
				label[labelLen - 1] = i - offset;
				labelType[labelLen - 1] = c;
			} else if (c == '@') {
				inString = 1;
				stringOffset = 0;
				label = GC_REALLOC(label, sizeof(int) * (++ labelLen));
				labelType = GC_REALLOC(labelType, sizeof(char) * labelLen);
				label[labelLen - 1] = i - offset;
				labelType[labelLen - 1] = c;
			} else if (c == ' ') {
				offset ++;
			} else if (c == '\n') {
				offset ++;
			} else {
				out->data[i-offset] = c;
			}
		}
		//printf("c: %c last: %c index: %d\n", c, out->data[i-offset], i-offset);
	}
	if (offset > 0) {
		out->data[(length-1)-offset] = 1 << 11;
	}
	//fread(out->data, sizeof(char), length, source);
	fclose(source);
	//gettimeofday(&timeEnd, NULL);
	//printf("read took: %luus\n", timeEnd.tv_usec - timeStart.tv_usec);
	//printf("len: %d datLen: %d\n", length, GC_size(out->data));
	//printf("%d ~ %d\n", &out->data[0], &out->data[1]);
	return out;
}

#endif
