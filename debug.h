#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <setjmp.h>
#include <stdarg.h>
#include <string.h>
#include "gc.h"
#include "bytecode.h"
#include "stack.h"
#include "error.h"
#include "io.h"
#include "opcodes.h"

// Debug Headers
int printOp(short c, Bytecode* main, long* currentLocation);
int opLength(short c, Bytecode* main, long* currentLocation);
int debugCLI(Bytecode* main, long* currentLocation, Stack* stack, Stack* localIndex, Stack* locals, Stack* localFrame);
void dumpStack(Stack* stack, FILE* stream);
void dumpFrame(Stack* localIndex, Stack* locals, Stack* localFrame, FILE* stream);
void dumpLocals(Stack* localIndex, Stack* locals, Stack* localFrame, FILE* stream);
// Debug Implementation

int debug = 0, interactive = 0;
int breaksLen = 0;
long* breaks = NULL;
int frameDepth = 0;

int printOp(short c, Bytecode* main, long* currentLocation) {
	int i = *currentLocation;
	if (((c & 0xFFFF) >> 8) == STRING_CONST) {
		char* val = (char*) (&main->data[i] + 1);
		int length = strlen(val);
		char* temp = malloc((length + 1) * sizeof(char));
		int mod = 0;
		for (int i = 0 ; i <= length ; i ++) {
			char c = val[i];
			if (c == '\n') {
				temp[i + mod] = '\\';
				temp = realloc(temp, (length + (++ mod) + 1) * sizeof(char));
				temp[i + mod] = 'n';
			} else if (c == '\t') {
				temp[i + mod] = '\\';
				temp = realloc(temp, (length + (++ mod) + 1) * sizeof(char));
				temp[i + mod] = 't';
			} else if (c == '\r') {
				temp[i + mod] = '\\';
				temp = realloc(temp, (length + (++ mod) + 1) * sizeof(char));
				temp[i + mod] = 'r';
			} else {
				temp[i + mod] = c;
			}
		}
		printf("String(\"%s\")\n", temp);
		free(temp);
		return (main->data[i] ^ (STRING_CONST << 8));
	}
	else if (((c & 0xFFFF) >> 8) == FUNCTION) { printf("Call(%s) (\n", (char*) (&main->data[i] + 1)); return (main->data[i] ^ (FUNCTION << 8)); }
	else if (((c & 0xFFFF) >> 8) == GET_VAR) { printf("Get(%s)\n", (char*) (&main->data[i] + 1)); return (main->data[i] ^ (GET_VAR << 8)); }
	else if (((c & 0xFFFF) >> 8) == SET_VAR) { printf("Set(%s)\n", (char*) (&main->data[i] + 1)); return (main->data[i] ^ (SET_VAR << 8)); }
	else if (((c & 0xFFFF) >> 8) == SYSTEM_EXIT) { printf("Exit()\n"); return (main->data[i] ^ (SYSTEM_EXIT << 8)); }
	else if (((c & 0xFFFF) >> 8) == INTEGER_CONST) { printf("Int(%d)\n", *((int*)&main->data[i+1])); return (main->data[i] ^ (INTEGER_CONST << 8)); }
	else if (((c & 0xFFFF) >> 8) == LIGHT_FRAME) { printf("Paren(+%d)(\n", (c & 0xFFFF) & (~(LIGHT_FRAME << 8))); }
	else if (((c & 0xFFFF) >> 8) == FRAME) { printf("Frame(+%d) (\n", (c & 0xFFFF) & (~(FRAME << 8))); }
	else if ((c & 0xFFFF) < ' ' || (c & 0xFFFF) > '~') printf("[%d]\n", (int) c);
	else printf("%c\n", (char) (c & 0xFFFF));
	return 0;
}
int opLength(short c, Bytecode* main, long* currentLocation) {
	int i = *currentLocation;
	if (((c & 0xFFFF) >> 7) == STRING_CONST) return (main->data[i] ^ (STRING_CONST << 8));
	else if (((c & 0xFFFF) >> 8) == 1) { return (main->data[i] ^ (FUNCTION << 8)) + 1; }
	else if (((c & 0xFFFF) >> 8) == 2) { return (main->data[i] ^ (GET_VAR << 8)) + 1; }
	else if (((c & 0xFFFF) >> 8) == 4) { return (main->data[i] ^ (SET_VAR << 8)) + 1; }
	else if (((c & 0xFFFF) >> 8) == 8) { return (main->data[i] ^ (SYSTEM_EXIT << 8)) + 1; }
	else if (((c & 0xFFFF) >> 8) == 16) { return (main->data[i] ^ (INTEGER_CONST << 8)) + 1; }
	else if (((c & 0xFFFF) >> 8) == 64) {  }
	else if (((c & 0xFFFF) >> 8) == 128) {  }
	return 1;
}

int debugCLI(Bytecode* main, long* currentLocation, Stack* stack, Stack* localIndex, Stack* locals, Stack* localFrame) {
	if (breaksLen > 0 && breaks[breaksLen-1] <= *currentLocation) {
		breaks = realloc(breaks, sizeof(long) * (-- breaksLen));
		printf("%4ld >", *currentLocation);
		char in[100];
		scanf("%s", (char*) &in);
		if (strcmp(in, "r") == 0) {
		} else if (strcmp(in, "bt") == 0) {
			dumpFrame(localIndex, locals, localFrame, stdout);
			breaks = realloc(breaks, sizeof(long) * (++ breaksLen));
			breaks[breaksLen-1] = *currentLocation;
			return 1;
		} else if (in[0] == 'b') {
			long num;
			sscanf(&in[1], "%ld", &num);
			breaks = realloc(breaks, sizeof(long) * (++ breaksLen));
			breaks[breaksLen-1] = num;
			return 1;
		} else if (strcmp(in, "?") == 0) {
			printf("\t?\tdisplay this help\n");
			printf("\tb #\tcreate a breakpoint\n");
			printf("\tr\trun to next breakpoint\n");
			printf("\ts\tbreak after next operation\n");
			printf("\tl\tlist next 5 operations\n");
			breaks = realloc(breaks, sizeof(long) * (++ breaksLen));
			breaks[breaksLen-1] = *currentLocation;
			return 1;
		} else if (strcmp(in, "s") == 0) {
			breaks = realloc(breaks, sizeof(long) * (++ breaksLen));
			breaks[breaksLen-1] = opLength(main->data[(*currentLocation)], main, currentLocation);
		} else if (strcmp(in, "l") == 0) {
			long pos = *currentLocation;
			for (int i = 0 ; i < 5 ; i ++) {
				printf("%4ld| ", pos);
				pos += printOp(main->data[pos], main, &pos);
				pos ++;
			}
			breaks = realloc(breaks, sizeof(long) * (++ breaksLen));
			breaks[breaksLen-1] = *currentLocation;
			return 1;
		} else if (strcmp(in, "d") == 0) {
			dumpFrame(localIndex, locals, localFrame, stdout);
			dumpLocals(localIndex, locals, localFrame, stdout);
			dumpStack(stack, stdout);
			breaks = realloc(breaks, sizeof(long) * (++ breaksLen));
			breaks[breaksLen-1] = *currentLocation;
			return 1;
		}
	}
	return 0;
}
void dumpStack(Stack* stack, FILE* stream) {
	fprintf(stream, "Stack:[");
	for (StackItem* item = stack->data ; item->type != Type_NULL ; item = item->next) {
		char* str = StackItem_toString(item);
		fprintf(stream, "%s", str);
		free(str);
		if (item->next->type != Type_NULL) {
			fprintf(stream, ", ");
		}
	}
	fprintf(stream, "]\n");
}
void dumpFrame(Stack* localIndex, Stack* locals, Stack* localFrame, FILE* stream) {
	fprintf(stream, "Frames:[");
	for (StackItem* item = localIndex->data ; item->type != Type_NULL ; item = item->next) {
		char* str = StackItem_toString(item);
		fprintf(stream, "%s", str);
		free(str);
		if (item->next->type != Type_NULL) {
			fprintf(stream, ", ");
		}
	}
	fprintf(stream, "]\n");
}
void dumpLocals(Stack* localIndex, Stack* locals, Stack* localFrame, FILE* stream) {
	fprintf(stream, "Locals:[");
	for (StackItem* item = locals->data ; item->type != Type_NULL ; item = item->next) {
		char* str = StackItem_toString(item);
		fprintf(stream, "%s", str);
		free(str);
		if (item->next->type != Type_NULL) {
			fprintf(stream, ", ");
		}
	}
	fprintf(stream, "]\n");
}

#endif // DEBUG_H
