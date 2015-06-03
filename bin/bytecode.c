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
		if (feof(source)) {
			length = i - 1;
			break;
		}
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
					printf("[INFO] character '%c' does not need to be delimited at line: %d\n", c, out->lineNumbers[out->lines - 1] - 1);
					*(((char*)&out->data[(i-offset)-stringOffset]) + stringOffset) = c;
				}
			} else {
				if (c == '\\') {
					delimiter = 1;
					stringOffset --;
					offset ++;
				} else if (c == '"' && labelType[labelLen - 1] == '"') {
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
				} else if (c == '(' && labelType[labelLen - 1] == '#') {
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
				} else if (c == '$' && labelType[labelLen - 1] == '$') {
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
				} else if (c == '@' && labelType[labelLen - 1] == '@') {
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
				} else if ((c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == ')' || c == ']') && labelLen > 0 && (labelType[labelLen-1] == '0' || labelType[labelLen-1] == '#' || labelType[labelLen - 1] == '/')) {
					if (labelType[labelLen - 1] == '#' || labelType[labelLen - 1] == '/') {
						labelType[labelLen - 1] == '/';
						if (c == '\n') {
							inString = 0;
							int start = label[labelLen - 1];
							while ((i-offset) >= start) {
								offset ++;
							}
							out->lines ++;
							out->lineNumbers = realloc(out->lineNumbers, out->lines * sizeof(long));
							out->lineNumbers[out->lines - 1] = i - offset;
							label = realloc(label, sizeof(int) * (-- labelLen));
							labelType = realloc(labelType, sizeof(char) * labelLen);
						} else {
						}
					} else {
						inString = 0;
						int start = label[labelLen - 1];
						char* num = ((char*)&out->data[start])+1;
						*(((char*)&out->data[(i-offset)-stringOffset]) + stringOffset) = (char) 0;
						int typeFloat = 0;
						for (char* dat = num ; *dat != (char) 0 ; dat ++)
							if (*dat == '.') {
								typeFloat = 1;
								break;
						} 
						if (typeFloat) {
							float* val = calloc(1, sizeof(float));
							sscanf(num, "%f", val);
							while ((i-offset)-start < sizeof(float) * 2) {
									offset --;
							}
							while ((i-offset)-start > sizeof(float) * 2) {
								offset ++;
							}
							*((int*)&out->data[start+1]) = *val;
							free(val);
						} else {
							int* val = calloc(1, sizeof(int));
							sscanf(num, "%d", val);
							while ((i-offset)-start < sizeof(int) * 2) {
									offset --;
							}
							while ((i-offset)-start > sizeof(int) * 2) {
								offset ++;
							}
							*((int*)&out->data[start+1]) = *val;
							free(val);
						}
						out->data[start] = ((i-offset)-start) | (INTEGER_CONST << 8);//1000
						label = realloc(label, sizeof(int) * (-- labelLen));
						labelType = realloc(labelType, sizeof(char) * labelLen);
						if (c == ')' && !funcParen && labelLen > 0 && labelType[labelLen - 1] == '(') {
							int start = label[labelLen - 1];
							out->data[start] = ((i-offset)-start) | (LIGHT_FRAME << 8);
							label = realloc(label, sizeof(int) * (-- labelLen));
							labelType = realloc(labelType, sizeof(char) * labelLen);
							out->data[i-offset] = c;
						} else if ( c == ')') {
							funcParen = 0;
							out->data[i-offset] = c;
						} else if (c == ']') {
							out->data[i-offset] = c;
						}
					}
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
				, labelLen);
		for (int j = 0 ; j < labelLen ; j ++) {
			printf("'%c'", labelType[j]);
			if (j != labelLen - 1) printf(", ");
		}
		printf("]\n");
		//*/
		if (i >= length - 1) {
			length *= 2;
			out->length = length;
			out->data = realloc(out->data, length * sizeof(short));
		}
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
			*((int*)&out->data[start+1]) = val;
			out->data[start] = (((length-1)-offset)-start) | (16 << 8);//1000
//debugOut(out, label, labelLen, labelType, length-1, offset, stringOffset);
			label = realloc(label, sizeof(int) * (-- labelLen));
			labelType = realloc(labelType, sizeof(char) * labelLen);
		}
	}
	offset -= 1;
	if (offset > 0) {
		out->data[(length-1)-offset] = 8 << 8;
		out->length = ((length-1)-offset)+1;
	}
	if (debug > 1) {
		printf("\naddress 0 1  2 3  4 5  6 7  8 9  A B  C D  E F  . .  . .    0123456789ABCDEF....\n");
		for (int i = 0 ; i < out->length ; i += 10) {
            int len = 0;
            char hex[50];
            memset(hex, 0, 50);
			printf("%6lX| ", i);//, &out->data[i]);
			for (int j = i ; j < i + 10 && j < out->length ; j ++) {
				short c = out->data[j];
				int b1 = (c >> 8) & 0xFF;
				int b2 = c & 0xFF;
				len += sprintf(hex+len, "%.2X%.2X ", b2, b1);
			}
			printf("%-50s  ", hex);
			for (int j = i ; j < i + 10 && j < out->length ; j ++) {
				short c = out->data[j];
				int b1 = (c >> 8) & 0xFF;
				int b2 = c & 0xFF;
				if (b1 > 0 && (b1 < ' ' || b1 > '~')) b1 = 176;
				if (b2 > 0 && (b2 < ' ' || b2 > '~')) b2 = 176;
				if (b1 == 0) b1 = (int) '.';
				if (b2 == 0) b2 = (int) '.';
				printf("%c%c", (char) b2, (char) b1);
			}
			printf("\n");
		}
		printf("\n");
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
		void* lib = dlopen("libsvm.debug.so", RTLD_LAZY);
		if (!lib) {
			errorMessage = "unable to find libsvm.debug.so";
			longjmp(errBuf, 1);
		}
		int (*_printOp)(short, Bytecode*, long*) = dlsym(lib, "printOp");
		if (dlerror()) {
			errorMessage = "unable to load libsvm.debug.so funcs";
			longjmp(errBuf, 1);
		}
		printf("Class: %s length: %ld\n", main->name, main->length);
		printf("--------------------.------.------.\n");
		printf(" BINARY             | HEX  | ADDR |\n");
		printf("--------------------+------+------|\n");
		int indent = 0;
		for (long i = 0 ; i < main->length ; i ++) {
			short c = main->data[i];
			for (int j = 15 ; j >= 0 ; j --) {
				printf("%c", (((c >> j) & 1) == 1) ? '1' : '0');
				if (j % 4 == 0) printf(" ");
			}
			printf("| %.4X | %5lX| ", (int) c & 0xFFFF, i);
			int length = 0;
			int opCode = (c & 0xFFFF) >> 8;
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
					short c = main->data[j];
					for (int j = 15 ; j >= 0 ; j --) {
						printf("%c", (((c >> j) & 1) == 1) ? '1' : '0');
						if (j % 4 == 0) printf(" ");
					}
					printf("| %.4X | %5lX|\n", (int) c & 0xFFFF, j);
				}
			}
			i += length;
		}
		printf("--------------------'------'------'\n");
		dlclose(lib);
	}
}
