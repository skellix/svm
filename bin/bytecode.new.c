#include "bytecode.h"

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/time.h>
#include "gc.h"
#include "error.h"
#include "opcodes.h"
#include "debug.h"
#include "stdfuncs.h"


#ifndef label_add

#define label_add(val, addr)\
	label = realloc(label, (++ labelLen) * sizeof(int));\
	labelType = realloc(labelType, labelLen * sizeof(char));\
	label[labelLen - 1] = addr;\
	labelType[labelLen - 1] = val;

#define label_remove()\
	label = realloc(label, (-- labelLen) * sizeof(int));\
	labelType = realloc(labelType, labelLen * sizeof(char));

#endif//label_add

Bytecode* Bytecode_loadFromFile(char* fileName, int debug) {
	FILE* source = fopen(fileName, "rb");
	if (source == NULL) {
		printf("Error: The source file could not be read!!!\n");
		exit(1);
	}
	int length = 32;
	Bytecode* out = malloc(sizeof(Bytecode));
	out->name = calloc(strlen(fileName) + 1, sizeof(char));
	strcpy(out->name, fileName);
	out->length = length;
	out->data = calloc(length, sizeof(char));
	out->lines = 0;
	out->lineNumbers = malloc(0);

	int c = -1;
	int labelLen = 1;
	int* label = malloc(0);
	char* labelType = calloc(labelLen, sizeof(char));
	labelType[0] = (char) 0;
	int delimiter = 0;
	int offset = 0;
	int i = 0;
	for (; i < length ; i ++) {
		if (feof(source)) {
			length = i - 1;
			break;
		}
		c = fgetc(source);
		if (labelType[labelLen-1] == '0' && c != '.' && (c < '0' || c > '9')) {
			while ((i-offset)-label[labelLen-1] < 5) {
				offset --;
			}
			out->data[i-offset] = (char) 0;
			char* valIn = &out->data[label[labelLen-1]];
			if (strchr(valIn, '.') == NULL) {
				int valOut = 0;
				sscanf(valIn, "%d", &valOut);
				*(int*) &out->data[label[labelLen-1]+1] = valOut;
			} else {
				float valOut = 0;
				sscanf(valIn, "%f", &valOut);
				out->data[label[labelLen-1]] = FLOAT_CONST;
				*(float*) &out->data[label[labelLen-1]+1] = valOut;
			}
			label_remove();
		}
		switch (c) {
			case '\\' : {
				if (labelType[labelLen-1] == '"') {
					if (delimiter) {
						out->data[i-offset] = '\\';
						delimiter = 0;
					} else {
						delimiter = 1;
						offset ++;
					}
				} else {
					out->data[i-offset] = (char) c;
				}
				break;}
			case '\n' : {
				offset ++;
				out->lines ++;
				out->lineNumbers = realloc(out->lineNumbers, out->lines * sizeof(long));
				out->lineNumbers[out->lines - 1] = (long) i - offset;
				break;}
			case '\r' : {
				break;}
			case '\t' : {
				if (labelType[labelLen-1] == '"') {
					out->data[i-offset] = (char) c;
				} else {
					offset ++;
				}
				break;}
			case '"' : {
				if (delimiter) {
					out->data[i-offset] = (char) c;
					delimiter = 0;
				} else if (labelType[labelLen-1] == '"') {
					*(int*) &out->data[label[labelLen-1]+1] = (i-offset) - label[labelLen-1];
					label_remove();
					out->data[i-offset] = (char) 0;
				} else {
					label_add(c, i - offset);
					out->data[i-offset] = STRING_CONST;
					offset -= 4;
				}
				break;}
			case '{' : {
					label_add(c, i - offset);
					out->data[i-offset] = FRAME;
					offset -= 4;
				break;}
			case '}' : {
				if (labelType[labelLen-1] == '{') {
					*(int*) &out->data[label[labelLen-1]+1] = (i-offset) - label[labelLen-1];
					label_remove();
					out->data[i-offset] = (char) c;
				} else {
					out->data[i-offset] = (char) c;
				}
				break;}
			case '$' : {
				if (labelType[labelLen-1] == '$') {
					*(int*) &out->data[label[labelLen-1]+1] = (i-offset) - label[labelLen-1];
					label_remove();
					out->data[i-offset] = (char) 0;
				} else {
					label_add(c, i - offset);
					out->data[i-offset] = GET_VAR;
					offset -= 4;
				}
				break;}
			case '@' : {
				if (labelType[labelLen-1] == '@') {
					*(int*) &out->data[label[labelLen-1]+1] = (i-offset) - label[labelLen-1];
					label_remove();
					out->data[i-offset] = (char) 0;
				} else {
					label_add(c, i - offset);
					out->data[i-offset] = SET_VAR;
					offset -= 4;
				}
				break;}
			case '#' : {
					label_add(c, i - offset);
					out->data[i-offset] = FUNCTION;
					offset -= 4;
				break;}
			case '(' : {
				if (labelType[labelLen-1] == '#') {
					*(int*) &out->data[label[labelLen-1]+1] = (i-offset) - label[labelLen-1];
					label_remove();
					out->data[i-offset] = (char) 0;
					label_add(')', i - offset);
				} else if (labelType[labelLen-1] == '"') {
					out->data[i-offset] = (char) c;
				} else {
					label_add(c, i - offset);
					out->data[i-offset] = LIGHT_FRAME;
					offset -= 4;
				}
				break;}
			case ')' : {
				if (labelType[labelLen-1] == ')') {
					label_remove();
					out->data[i-offset] = (char) c;
				} else if (labelType[labelLen-1] == '(') {
					*(int*) &out->data[label[labelLen-1]+1] = (i-offset) - label[labelLen-1];
					label_remove();
					out->data[i-offset] = (char) c;
				} else {
					out->data[i-offset] = (char) c;
				}
				break;}
			case '0' :
			case '1' :
			case '2' :
			case '3' :
			case '4' :
			case '5' :
			case '6' :
			case '7' :
			case '8' :
			case '9' : {
				char last = labelType[labelLen-1];
				if (last == '"' || last == '#' || last == '$' || last == '@') {
					out->data[i-offset] = (char) c;
				} else if (labelType[labelLen-1] != '0') {
					label_add('0', i - offset)
					out->data[i-offset] = INTEGER_CONST;
					offset --;
					out->data[i-offset] = (char) c;
				} else {
					out->data[i-offset] = (char) c;
				}
				break;}
			default : {
				if (delimiter) {
					if (c == 'n') {
						out->data[i-offset] = '\n';
					} else if (c == 'r') {
						out->data[i-offset] = '\r';
					} else if (c == 't') {
						out->data[i-offset] = '\t';
					}
					delimiter = 0;
				} else if (c == ' ') {
					if (labelType[labelLen-1] == '#') {
						while (i-offset > label[labelLen - 1])
							offset ++;
						while (fgetc(source) != '\n')
							offset ++;
					}
					offset ++;
				} else if (c == '!' && i == 1 && labelType[labelLen-1] == '#') {
					label_remove();
					offset -= 6;
					while (fgetc(source) != '\n') offset ++;
				} else {
					out->data[i-offset] = (char) c;
				}
				break;}
		}
		if (i - offset >= length - 6) {
			length *= 2;
			out->length = length;
			out->data = realloc(out->data, length * sizeof(char));
		}
	}
	offset -= 1;
	out->data[(length-1)-offset] = SYSTEM_EXIT;
	length = i - offset	- 1;
	out->length = length;
	out->data = realloc(out->data, length * sizeof(char));
	if (debug > 1) {
		printf("\naddress 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F    0123456789ABCDEF\n");
		for (int i = 0 ; i < out->length ; i += 16) {
            int len = 0;
            char hex[50];
            memset(hex, 0, 50);
			printf("%6lX| ", i);
			for (int j = i ; j < i + 16 && j < out->length ; j ++) {
				char c = out->data[j];
				int b1 = c & 0xFF;
				len += sprintf(hex+len, "%.2X ", b1);
			}
			printf("%-50s", hex);
			for (int j = i ; j < i + 16 && j < out->length ; j ++) {
				char c = out->data[j];
				int b1 = c & 0xFF;
				if (b1 > 0 && (b1 < ' ' || b1 > '~')) b1 = 176;
				if (b1 == 0) b1 = (int) '.';
				printf("%c", (char) b1);
			}
			printf("\n");
		}
		printf("\n");
		outputOps(out);
	}
	fclose(source);
	free(label);
	free(labelType);
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

void outputOps(Bytecode* main) {
	if (debug > 2) {
		int (*_printOp)(short, Bytecode*, long*) = dlsym(debugLib, "printOp");
		if (dlerror()) {
			errorMessage = "unable to load libsvm.debug.so funcs";
			longjmp(errBuf, 1);
		}
		printf("Class: %s length: %ld\n", main->name, main->length);
		printf("----------.-----.------.\n");
		printf(" BINARY   | HEX | ADDR |\n");
		printf("----------+-----+------|\n");
		int indent = 0;
		for (long i = 0 ; i < main->length ; i ++) {
			char c = main->data[i];
			for (int j = 7 ; j >= 0 ; j --) {
				printf("%c", (((c >> j) & 1) == 1) ? '1' : '0');
				if (j % 4 == 0) printf(" ");
			}
			printf("| %.2X  | %5lX| ", (int) c & 0xFF, i);
			int length = 0;
			int opCode = c & 0xFF;
			if (opCode == FUNCTION || opCode == LIGHT_FRAME || opCode == FRAME) {
				for (int j = indent ; j > 0 ; j --) printf("  ");
				length = _printOp(c, main, &i);
				indent ++;
			} else if ((char) c == ')' || (char) c == '}') {
			    indent --;
				for (int j = indent ; j > 0 ; j --) printf("  ");
				printf("%c\n", (char) c);
			} else {
				for (int j = indent ; j > 0 ; j --) printf("  ");
					length = _printOp(c, main, &i);
				}
			if (debug > 3) {
				for (int j = i + 1 ; j <= i + length ; j ++) {
					char c = main->data[j];
					for (int j = 7 ; j >= 0 ; j --) {
						printf("%c", (((c >> j) & 1) == 1) ? '1' : '0');
						if (j % 4 == 0) printf(" ");
					}
					printf("| %.2X  | %5lX| [%c]\n", (int) c & 0xFF, j, c);
				}
			}
			i += length;
		}
		printf("----------'-----'------'\n");
	}
}
