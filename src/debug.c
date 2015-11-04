#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdarg.h>
#include <string.h>
//#include "gc.h"
#include "bytecode.h"
#include "stack.h"
//#include "error.h"
#include "io.h"
#include "opcodes.h"
#include "stdfuncs.h"

int printOp(short c, Bytecode* main, long* currentLocation) {
	int i = *currentLocation;
	switch (main->data[i] & 0xFF) {
		case STRING_CONST : {
			char* val = &main->data[i + 5];
			int length = *(int*) (&main->data[i + 1]);
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
			return *(int*) &main->data[i+1];}
		case FUNCTION : {
			printf("Call(%s) (\n",  (&main->data[i + 5]));
			return *(int*) &main->data[i+1];}
		case GET_VAR : {
			printf("Get(%s)\n", (&main->data[i + 5]));
			return *(int*) &main->data[i+1];}
		case SET_VAR : {
			printf("Set(%s)\n", (&main->data[i + 5]));
			return *(int*) &main->data[i+1];}
		case SYSTEM_EXIT : {
			printf("Exit()\n");
			return 0;}
		case INTEGER_CONST : {
			printf("Int(%d)\n", *(int*) &main->data[i+1] & 0xFFFF);
			return 4;}
		case FLOAT_CONST : {
			printf("Float(%f)\n", *(float*) &main->data[i+1]);
			return 4;}
		case LIGHT_FRAME : {
			printf("Paren(+%d)(\n", *(int*) &main->data[i+1] & 0xFFFF);
			return 4;}
		case FRAME : {
			printf("Frame(+%d) {\n", *(int*) &main->data[i+1] & 0xFFFF);
			return 4;}
		default : {
			if (main->data[i] < ' ' || main->data[i] > '~') printf("[%d]\n", (int) main->data[i]);
			else printf("%c\n", main->data[i]);}
	}
	return 0;
}

int opLength(short c, Bytecode* main, long* currentLocation) {
	int i = *currentLocation;
	switch (c & 0xFF) {
		case STRING_CONST :
		case FUNCTION :
		case GET_VAR :
		case SET_VAR : return *(int*) &main->data[i + 1];
		case INTEGER_CONST : return 4;
		default : return 1;
	}
	return 1;
}

int debugCLI(Bytecode* main, long* currentLocation, Stack* stack, Stack* locals, Stack* localFrame) {
	if (breaksLen > 0 && breaks[breaksLen-1] <= *currentLocation) {
		breaks = realloc(breaks, sizeof(long) * (-- breaksLen));
		printf("%6lX >", *currentLocation);
		char in[100];
		scanf("%s", (char*) &in);
		if (strcmp(in, "r") == 0) {
		} else if (strcmp(in, "bt") == 0) {
			dumpFrame(locals, localFrame, stdout);
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
			printf("\td\tdump the thread info\n");
			printf("\tq\texit the debugger\n");
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
			dumpFrame(locals, localFrame, stdout);
			dumpLocals(locals, localFrame, stdout);
			dumpStack(stack, stdout);
			breaks = realloc(breaks, sizeof(long) * (++ breaksLen));
			breaks[breaksLen-1] = *currentLocation;
			return 1;
		} else if (strcmp(in, "q") == 0) {
			exit(0);
		}
	}
	return 0;
}


