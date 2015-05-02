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
	short* data;
	int lines;
	long* lineNumbers;
} Bytecode;

void Bytecode_dispose(Bytecode* code) {
	//printf("disposing bytecode: %10lX\n", code);
	free(code->data);
	free(code->lineNumbers);
	free(code->name);
	free(code);
}

void debugOut(Bytecode* out, int* label, int labelLen, char* labelType, int i, int offset, int stringOffset);

Bytecode* Bytecode_loadFromFile(char* fileName, int debug) {
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
	Bytecode* out = malloc(sizeof(Bytecode));
	out->name = malloc((strlen(fileName) + 1) * sizeof(char));
	strcpy(out->name, fileName);
	out->length = length;
	out->data = malloc(sizeof(short) * length);
	for (int i = 0 ; i < out->length ; i ++) {
		out->data[i] = (short) 0;
	}
	out->lines = 0;
	out->lineNumbers = malloc(sizeof(short));

	int c = -1;
	int labelLen = 0;
	int* label = malloc(sizeof(int));
	char* labelType = malloc(sizeof(char));
	int delimiter = 0;
	int offset = 0;
	int inString = 0;
	int funcParen = 0;
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
					printf("character '%c' does not need to be delimited!!!\n", c);
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
					int length = (((char*)&out->data[(i-offset)-stringOffset]) + stringOffset) - (char*)&out->data[start+1];
					int newEnd = start + 1 + (length / 2);
					offset += (i-offset) - newEnd;
					out->data[start] = ((i-offset)-start) | (STRING_CONST << 8);//0080
//debugOut(out, label, labelLen, labelType, i, offset, stringOffset);
					label = realloc(label, sizeof(int) * (-- labelLen));
					labelType = realloc(labelType, sizeof(char) * labelLen);
				} else if (c == '(') {
					inString = 0;
					funcParen = 1;
					*(((char*)&out->data[(i-offset)-stringOffset]) + stringOffset) = (char) 0;
					int start = label[labelLen - 1];
					int length = (((char*)&out->data[(i-offset)-stringOffset]) + stringOffset) - (char*)&out->data[start+1];
					int newEnd = start + 1 + (length / 2);
					offset += (i-offset) - newEnd;
					out->data[start] = ((newEnd)-start) | (FUNCTION << 8);//0100
//debugOut(out, label, labelLen, labelType, i, offset, stringOffset);
					label = realloc(label, sizeof(int) * (-- labelLen));
					labelType = realloc(labelType, sizeof(char) * labelLen);
				} else if (c == '$') {
					inString = 0;
					*(((char*)&out->data[(i-offset)-stringOffset]) + stringOffset) = (char) 0;
					int start = label[labelLen - 1];
					int length = (((char*)&out->data[(i-offset)-stringOffset]) + stringOffset) - (char*)&out->data[start+1];
					int newEnd = start + 1 + (length / 2);
					offset += (i-offset) - newEnd;
					out->data[start] = ((newEnd)-start) | (GET_VAR << 8);//0200
//debugOut(out, label, labelLen, labelType, i, offset, stringOffset);
					label = realloc(label, sizeof(int) * (-- labelLen));
					labelType = realloc(labelType, sizeof(char) * labelLen);
				} else if (c == '@') {
					inString = 0;
					*(((char*)&out->data[(i-offset)-stringOffset]) + stringOffset) = (char) 0;
					int start = label[labelLen - 1];
					int length = (((char*)&out->data[(i-offset)-stringOffset]) + stringOffset) - (char*)&out->data[start+1];
					int newEnd = start + 1 + (length / 2);
					offset += (i-offset) - newEnd;
					out->data[start] = ((newEnd)-start) | (SET_VAR << 8);//0400
//debugOut(out, label, labelLen, labelType, i, offset, stringOffset);
					label = realloc(label, sizeof(int) * (-- labelLen));
					labelType = realloc(labelType, sizeof(char) * labelLen);
				} else if ((c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == ')') && labelLen > 0 && labelType[labelLen-1] == '0') {
					inString = 0;
					int start = label[labelLen - 1];
					while ((i-offset)-start < 2) {
						offset --;
					}
					while ((i-offset)-start > 2) {
						offset ++;
					}
//debugOut(out, label, labelLen, labelType, i, offset, stringOffset);
					char* num = ((char*)&out->data[start])+1;
					int val;
					sscanf(num, "%d", &val);
					*((int*)&out->data[start+1]) = val;
					out->data[start] = ((i-offset)-start) | (INTEGER_CONST << 8);//1000
//debugOut(out, label, labelLen, labelType, i, offset, stringOffset);
					label = realloc(label, sizeof(int) * (-- labelLen));
					labelType = realloc(labelType, sizeof(char) * labelLen);
//				} else if (c == ' ' && labelType[labelLen - 1] == '!') {
//					inString = 0;
//					int start = label[labelLen - 1];
//					out->data[start] = '!';
//					label = GC_REALLOC(label, sizeof(int) * (-- labelLen));
//					labelType = GC_REALLOC(labelType, sizeof(char) * labelLen);
//					offset ++;
				} else {
					*(((char*)&out->data[(i-offset)-stringOffset]) + stringOffset) = (char) c;
//debugOut(out, label, labelLen, labelType, i, offset, stringOffset);
				}
			}
			stringOffset ++;
		} else {
			if (c == '"') {
				inString = 1;
				stringOffset = 0;
				label = realloc(label, sizeof(int) * (++ labelLen));
				labelType = realloc(labelType, sizeof(char) * labelLen);
				label[labelLen - 1] = i - offset;
				labelType[labelLen - 1] = c;
			} else if (c == '#') {
				inString = 1;
				stringOffset = 0;
				label = realloc(label, sizeof(int) * (++ labelLen));
				labelType = realloc(labelType, sizeof(char) * labelLen);
				label[labelLen - 1] = i - offset;
				labelType[labelLen - 1] = c;
			} else if (c == '$') {
				inString = 1;
				stringOffset = 0;
				label = realloc(label, sizeof(int) * (++ labelLen));
				labelType = realloc(labelType, sizeof(char) * labelLen);
				label[labelLen - 1] = i - offset;
				labelType[labelLen - 1] = c;
			} else if (c == '@') {
				inString = 1;
				stringOffset = 0;
				label = realloc(label, sizeof(int) * (++ labelLen));
				labelType = realloc(labelType, sizeof(char) * labelLen);
				label[labelLen - 1] = i - offset;
				labelType[labelLen - 1] = c;
			} else if (c == '{') {
				label = realloc(label, sizeof(int) * (++ labelLen));
				labelType = realloc(labelType, sizeof(char) * labelLen);
				label[labelLen - 1] = i - offset;
				labelType[labelLen - 1] = c;
			} else if (c == '}') {
				int start = label[labelLen - 1];
				out->data[start] = ((i-offset)-start) | (FRAME << 8);
				label = realloc(label, sizeof(int) * (-- labelLen));
				labelType = realloc(labelType, sizeof(char) * labelLen);
				out->data[i-offset] = c;
			} else if (c == '(') {
				label = realloc(label, sizeof(int) * (++ labelLen));
			labelType = realloc(labelType, sizeof(char) * labelLen);
				label[labelLen - 1] = i - offset;
				labelType[labelLen - 1] = c;
			} else if (c == ')' && !funcParen && labelLen > 0 && labelType[labelLen - 1] == '(') {
				int start = label[labelLen - 1];
				out->data[start] = ((i-offset)-start) | (LIGHT_FRAME << 8);
				label = realloc(label, sizeof(int) * (-- labelLen));
				labelType = realloc(labelType, sizeof(char) * labelLen);
				out->data[i-offset] = c;
			} else if (c == ')') {
				funcParen = 0;
				out->data[i-offset] = c;
			} else if (c >= '0' && c <= '9') {
				inString = 1;
				stringOffset = 0;
				label = realloc(label, sizeof(int) * (++ labelLen));
				labelType = realloc(labelType, sizeof(char) * labelLen);
				label[labelLen - 1] = i - offset;
				labelType[labelLen - 1] = '0';
				*(((char*)&out->data[(i-offset)-stringOffset]) + stringOffset+1) = (char) c;
			} else if (c == ' ') {
				offset ++;
			} else if (c == '\n') {
				offset ++;
				out->lines ++;
				out->lineNumbers = realloc(out->lineNumbers, out->lines * sizeof(long));
				out->lineNumbers[out->lines - 1] = i - offset;
			} else if (c == '\t') {
				offset ++;
			} else if (c == '\r') {
				offset ++;
			} else {
				out->data[i-offset] = c;
			}
		}
		/*
		printf("c: %c last: %c index: %d labelLen: %d labelType: ["
				, c
				, out->data[i-offset]
				, i-offset
				,labelLen);
		for (int j = 0 ; j < labelLen ; j ++) {
			printf("'%c'", labelType[j]);
			if (j != labelLen - 1) printf(", ");
		}
		printf("]\n");
		//*/
	}
	if (inString) {
		if (labelLen > 0 && labelType[labelLen-1] == '0') {
			inString = 0;
			int start = label[labelLen - 1];
			while (((length-1)-offset)-start < 2) {
				offset --;
			}
//debugOut(out, label, labelLen, labelType, length-1, offset, stringOffset);
			char* num = ((char*)&out->data[start])+1;
			int val;
			sscanf(num, "%d", &val);
//printf("int read: %d\n", val);
			*((int*)&out->data[start+1]) = val;
			out->data[start] = (((length-1)-offset)-start) | (16 << 8);//1000
			//out->data[start] = (16 << 8);//1000
//debugOut(out, label, labelLen, labelType, length-1, offset, stringOffset);
			label = realloc(label, sizeof(int) * (-- labelLen));
			labelType = realloc(labelType, sizeof(char) * labelLen);
		}
	}
	offset -= 1;
	if (offset > 0) {
		out->data[(length-1)-offset] = 8 << 8;
		out->length = ((length-1)-offset)+1;
		//out->data = realloc(out->data, out->length * sizeof(short));
	}
	if (debug > 1) {
		printf("\naddress 0 1  2 3  4 5  6 7  8 9    0123456789\n");
		for (int i = 0 ; i < out->length ; i += 5) {
            int len = 0;
            char hex[25];
            memset(hex, 0, 25);
			printf("%6lX| ", i);//, &out->data[i]);
			for (int j = i ; j < i + 5 && j < out->length ; j ++) {
				short c = out->data[j];
				int b1 = (c >> 8) & 0xFF;
				int b2 = c & 0xFF;
				len += sprintf(hex+len, "%.2X%.2X ", b2, b1);
				//printf("%0.2X%0.2X ", b2, b1);
			}
			printf("%-25s  ", hex);
			for (int j = i ; j < i + 5 && j < out->length ; j ++) {
				short c = out->data[j];
				int b1 = (c >> 8) & 0xFF;
				int b2 = c & 0xFF;
				if (b1 > 0 && (b1 < ' ' || b1 > '~')) b1 = (char) 176;
				if (b2 > 0 && (b2 < ' ' || b2 > '~')) b2 = (char) 176;
				if (b1 == 0) b1 = '.';
				if (b2 == 0) b2 = '.';
				printf("%c%c", (char) b2, (char) b1);
			}
			printf("\n");
		}
		printf("\n");
	}
	//fread(out->data, sizeof(char), length, source);
	fclose(source);
	free(label);
	free(labelType);
	//gettimeofday(&timeEnd, NULL);
	//printf("read took: %luus\n", timeEnd.tv_usec - timeStart.tv_usec);
	//printf("len: %d datLen: %d\n", length, GC_size(out->data));
	//printf("%d ~ %d\n", &out->data[0], &out->data[1]);
	return out;
}

void debugOut(Bytecode* out, int* label, int labelLen, char* labelType, int i, int offset, int stringOffset) {
	for (char* j = (char*)&out->data[label[labelLen - 1]] ; j < (char*)&out->data[i-offset+1] ; j ++) {
		char jVal = *j;
		printf("   ");
		if (j == (((char*)&out->data[(i-offset)-stringOffset]) + stringOffset)) {
			printf(">");
		} else if (j == (char*)&out->data[label[labelLen - 1]]) {
			printf("{");
		} else {
			printf(" ");
		}
		printf("%4p: %4d ", (void*) j, (int) *j);
		if (jVal >= ' ' && jVal <= '~') {
			printf("'%c'", jVal);
		}
		printf("\n");
	}
	printf("    sdata: %c length: %d\n"
		, *(((char*)&out->data[(i-offset)-stringOffset]) + stringOffset)
		, (i-offset)-(label[labelLen - 1]));
}

#endif
