#!/usr/local/bin/tcc -run -g -L/usr/lib -lgc

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <gc/gc.h>
#include "bytecode.h"
#include "stack.h"

void showHelp(){
	fprintf(stderr, "USAGE: foo <input>\n");
}

int debug = 0;

int runBytecode(Bytecode* main, Stack* stack, long* lineNumber);
void dumpStack(Stack* stack, FILE* stream);
int main(int argc, char** argv) {
	if (argc <= 1) {
		showHelp(); return 0;
	}
	for (int i = 1 ; i < argc ; i ++) {
		if (strcmp(argv[i], "-X") == 0) {
			debug ++;
		}
	}
	GC_init();
	Bytecode* main = Bytecode_loadFromFile(argv[argc - 1]);
	Stack* stack = Stack_create();
	long lineNumber = 0;
	int exitStatus = -1;
	if ((exitStatus = runBytecode(main, stack, &lineNumber)) != 0) {
		fprintf(stderr, "Error %d at line %d:\n", exitStatus, lineNumber);
		dumpStack(stack, stderr);
		return 1;
	}
	return 0;
}
void exec(Bytecode* main, Stack* stack, int* running, long* currentLocation, long* lineNumber);
int runBytecode(Bytecode* main, Stack* stack, long* lineNumber) {
	if (debug) {
		printf("Class: %s length: %d\n", main->name, main->length);
		for (int i = 0 ; i < main->length ; i ++) {
			printf("%c", main->data[i]);
		}
		printf("\n#####\n");
	}
	int running = 1;
	long currentLocation = 0;
	while (running) {
		exec(main, stack, &running, &currentLocation, lineNumber);
		do {
			currentLocation ++;
			if (currentLocation > main->length) {
				running = 0;
				if (stack->length > 0) {
					StackItem* item = Stack_pop(stack);
					if (item->type == Type_INT) {
						return *((int*)item->item);
					}
				}
			}
			if (main->data[currentLocation] == '\n') {
				(*lineNumber) ++;
			}
		} while (main->data[currentLocation] == '\n');
	}
	return 0;
}
void exec(Bytecode* main, Stack* stack, int* running, long* currentLocation, long* lineNumber) {
	if (debug > 1) {
		printf("command: '%c'\n", main->data[(*currentLocation)]);
	}
	if (main->data[(*currentLocation)] >= '0'
			&& main->data[(*currentLocation)] <= '9') {
		long start = (*currentLocation);
		//printf("found function call at %d\n", currentLocation);
		long end = -1;
		int decimal = 0;
		for (long i = start ; i < main->length ; i ++) {
			if (main->data[i] < '0' || main->data[i] > '9') {
				if (main->data[i] == '.') {
					decimal = 1;
				} else {
					end = i;
					break;
				}
			}
		}
		if (end == -1) {
			fprintf(stderr, "A function name was not specified at: -\n");
			return 1;
		}
		(*currentLocation) = end - 1;
		const long length = end - start;
		char* numStr = GC_MALLOC(sizeof(char) * (length + 1));
		for (long i = 0 ; i < length ; i ++) {
			numStr[i] = main->data[start + i];
		}
		numStr[length] = 0;
		if (decimal) {
			float* val = GC_MALLOC(sizeof(float));
			sscanf(numStr, "%f", val);
			Stack_push(stack, newStackItem(Type_FLOAT, val));
		} else {
			int* val = GC_MALLOC(sizeof(int));
			sscanf(numStr, "%d", val);
			Stack_push(stack, newStackItem(Type_INT, val));
		}
		GC_free(numStr);
		//printf("adding %d to the stack\n", *val);
	} else if (main->data[(*currentLocation)] == '+') {
		StackItem* rOperand = Stack_pop(stack);
		StackItem* lOperand = Stack_pop(stack);
		Type oType = rOperand->type > lOperand->type ? rOperand->type : lOperand->type;
		if (oType == Type_STRING) {
			fprintf(stderr, "strings types can't be used in math!!!\n");
			exit(1);
		}
		if (oType == Type_CHAR) {
			char* val = GC_MALLOC(sizeof(char));
			*val = (char) *rOperand->item + (char) *lOperand->item;
			Stack_push(stack, newStackItem(oType, val));
		} else if (oType == Type_INT) {
			int* val = GC_MALLOC(sizeof(int));
			*val = (int) *rOperand->item + (int) *lOperand->item;
			Stack_push(stack, newStackItem(oType, val));
		} else if (oType == Type_FLOAT) {
			float* val = GC_MALLOC(sizeof(float));
			fprintf(stderr, "casting int to float is not yet supported on this system!!!\n");
			exit(1);
			/*
			*val = 
				rOperand->type == Type_INT || rOperand->type == Type_CHAR ? (float) *((int*)rOperand->item) : (float) *((float*)rOperand->item)
				+ lOperand->type == Type_INT || lOperand->type == Type_CHAR ? (float) *((int*)lOperand->item) : (float) *((float*)lOperand->item);
			*/
			Stack_push(stack, newStackItem(oType, val));
		}
	} else if (main->data[(*currentLocation)] == '(') {
		long label = stack->length;
		(*currentLocation) ++;
		while (main->data[(*currentLocation)] != ')') {
			exec(main, stack, running, currentLocation, lineNumber);
			(*currentLocation) ++;
		}
		while (stack->length > label + 1) {
			StackItem* item = Stack_pop(stack);
			GC_free(item->item);
			GC_free(item);
		}
	} else if (main->data[(*currentLocation)] == '!') {
		long start = (*currentLocation) + 1;
		//printf("found function call at %d\n", currentLocation);
		long end = -1;
		for (long i = start ; i < main->length ; i ++) {
			if (main->data[i] == '(') {
				end = i;
				break;
			}
		}
		if (end == -1) {
			fprintf(stderr, "A function name was not specified at: -\n");
			return 1;
		}
		const long length = end - start;
		char* funcName = GC_MALLOC(sizeof(char) * (length + 1));
		for (long i = 0 ; i < length ; i ++) {
			funcName[i] = main->data[start + i];
		}
		funcName[length] = 0;
		//printf("function name is: %s\n", funcName);
		long label = stack->length;
		(*currentLocation) = end + 1;
		while (main->data[(*currentLocation)] != ')') {
			exec(main, stack, running, currentLocation, lineNumber);
			(*currentLocation) ++;
		}
		if (debug) {
			printf("called function '%s'\n", funcName);
		}
		if (strcmp(funcName, "print") == 0) {
			Stack* argStack = Stack_create();
			while (stack->length > label) {
				Stack_push(argStack, Stack_pop(stack));
			}
			StackItem* format = Stack_pop(argStack);
			if (format->type == Type_STRING) {
				int start1 = 0, end1 = -1;
				char* fData = ((char*)format->item);
				int fLen = strlen(fData);
				int arg = 0;
				for (int i = 0 ; i <= fLen ; i ++) {
					if (fData[i] == '%' || i == fLen) {
						end1 = i;
						int len = end1 - start1;
						char* temp = GC_MALLOC(sizeof(char) * (len + 1));
						for (int i = 0 ; i < len ; i ++) {
							temp[i] = fData[start1 + i];
						}
						//snprintf(fData + start1, len, "%s", temp);
						temp[len] = 0;
						if (arg) {
							StackItem* item = Stack_pop(argStack);
							printf(temp, 
								item->type == Type_CHAR ? *((char*)item->item) :
								item->type == Type_INT ? *((int*)item->item) :
								//item->type == Type_FLOAT ? *((float*)item->item) :
								item->type == Type_STRING ? ((char*)item->item) : NULL
							);
						} else {
							printf(temp);
							arg = 1;
						}
						GC_free(temp);
						start1 = i;
					}
				}
			}
		} else if (strcmp(funcName, "dumpStack") == 0) {
			dumpStack(stack, stdout);
			while (stack->length > label) {
				StackItem* item = Stack_pop(stack);
				GC_free(item->item);
				GC_free(item);
			}
		}
		//printf("next command will be: '%c'\n", main->data[(*currentLocation)]);
	} else if (main->data[(*currentLocation)] == '"') {
		long start = (*currentLocation) + 1;
		//printf("found string at %d\n", currentLocation);
		long end = -1;
		for (long i = start ; i < main->length ; i ++) {
			if (main->data[i] == '"') {
				end = i;
				break;
			}
		}
		if (end == -1) {
			fprintf(stderr, "A function name was not specified at: -\n");
			return 1;
		}
		const long length = end - start;
		char* stringData = GC_MALLOC(sizeof(char) * (length - 1));
		int escaped = 0, offset = 0;
		for (long i = 0 ; i < length; i ++) {
			char c = main->data[start + i];
			if (c == '\\') {
				escaped = 1;
				offset ++;
			} else if (escaped) {
				escaped = 0;
				if (c == 'n') {
					stringData[i - offset] = '\n';
				} else if (c == 't') {
				} else {
					stringData[i - offset] = '\t';
				}
			} else {
				stringData[i - offset] = c;
			}
		}
		stringData[length] = 0;
		Stack_push(stack, newStackItem(Type_STRING, stringData));
		(*currentLocation) = end;
	}
}
void dumpStack(Stack* stack, FILE* stream) {
	fprintf(stream, "Stack:[");
	for (int i = 0 ; i < stack->length ; i ++) {
		StackItem* item = stack->data[i];
		fprintf(stream, "%s", StackItem_toString(item));
		if (i < stack->length - 1) {
			fprintf(stream, ", ");
		}
	}
	fprintf(stream, "]\n");
}
