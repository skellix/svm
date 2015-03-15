#!/usr/local/bin/tcc -run -g -I~/workspace/test10 -L/usr/lib -lgc

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
int error = 0;
char* errorMessage = NULL;

int runBytecode(Bytecode* main, Stack* stack, Stack* localIndex, Stack* locals, Stack* localFrame, long* lineNumber);
void dumpStack(Stack* stack, FILE* stream);
void dumpLocals(Stack* localIndex, Stack* locals, Stack* localFrame, FILE* stream);
void setError(char* message);
int main(int argc, char** argv) {
	if (argc <= 1) {
		showHelp(); return 0;
	}
	int norun = 0;
	for (int i = 1 ; i < argc ; i ++) {
		if (strcmp(argv[i], "-E") == 0) {
			norun = 1;
		}
		if (strcmp(argv[i], "-X") == 0) {
			debug ++;
		}
	}
	GC_init();
	Bytecode* main = Bytecode_loadFromFile(argv[argc - 1]);
	if (debug) {
		printf("Class: %s length: %d\n", main->name, main->length);
		for (int i = 0 ; i < main->length ; i ++) {
			short c = main->data[i];
			if ((c >> 7) & 1) { printf("%4d: String(\"%s\")\n", i, (char*) (&main->data[i] + 1)); i += (main->data[i] ^ (1 << 7)); }
			else if ((c >> 8) & 1) { printf("%4d: Call(%s)\n", i, (char*) (&main->data[i] + 1)); i += (main->data[i] ^ (1 << 8)); }
			else if ((c >> 9) & 1) { printf("%4d: Get(%s)\n", i, (char*) (&main->data[i] + 1)); i += (main->data[i] ^ (1 << 9)); }
			else if ((c >> 10) & 1) { printf("%4d: Set(%s)\n", i, (char*) (&main->data[i] + 1)); i += (main->data[i] ^ (1 << 10)); }
			else if ((c >> 11) & 1) { printf("%4d: Exit()\n", i); i += (main->data[i] ^ (1 << 11)); }
			else printf("%4d: %c\n", i, (char) c);
			//printf("%c", main->data[i]);
		}
		printf("\n#####\n");
	}
	if (!norun) {
		Stack* stack = Stack_create();
		Stack* localIndex = Stack_create();
		Stack* locals = Stack_create();
		Stack* localFrame = Stack_create();
		long lineNumber = 0;
		int exitStatus = -1;
		if ((exitStatus = runBytecode(main, stack, localIndex, locals, localFrame, &lineNumber)) != 0) {
			if (error) {
				fprintf(stderr, "Error %s at line %d:\n", errorMessage, lineNumber);
			} else {
				fprintf(stderr, "Error %d at line %d:\n", exitStatus, lineNumber);
			}
			dumpLocals(localIndex, locals, localFrame, stderr);
			dumpStack(stack, stderr);
			return 1;
		}
	}
	return 0;
}
void exec(Bytecode* main, Stack* stack, Stack* localIndex, Stack* locals, Stack* localFrame, int* running, long* currentLocation, long* lineNumber);
int runBytecode(Bytecode* main, Stack* stack, Stack* localIndex, Stack* locals, Stack* localFrame, long* lineNumber) {
	int running = 1;
	long currentLocation = 0;
	while (running) {
		exec(main, stack, localIndex, locals, localFrame, &running, &currentLocation, lineNumber);
		if (error) {
			return 1;
		}
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
// Interrupts
void int_printf(argStack);
StackItem* int_add(StackItem* lOperand, StackItem* rOperand);
StackItem* int_subtract(StackItem* lOperand, StackItem* rOperand);
StackItem* int_multiply(StackItem* lOperand, StackItem* rOperand);
StackItem* int_divide(StackItem* lOperand, StackItem* rOperand);

void exec(Bytecode* main, Stack* stack, Stack* localIndex, Stack* locals, Stack* localFrame, int* running, long* currentLocation, long* lineNumber) {
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
			if ((char) main->data[i] < '0' || main->data[i] > '9') {
				if ((char) main->data[i] == '.') {
					decimal = 1;
				} else {
					end = i;
					break;
				}
			}
		}
		if (end == -1) {
			fprintf(stderr, "An invalid number was found at: -\n");
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
		Stack_push(stack, int_add(lOperand, rOperand));
	} else if (main->data[(*currentLocation)] == '-') {
		StackItem* rOperand = Stack_pop(stack);
		StackItem* lOperand = Stack_pop(stack);
		Stack_push(stack, int_subtract(lOperand, rOperand));
	} else if (main->data[(*currentLocation)] == '*') {
		StackItem* rOperand = Stack_pop(stack);
		StackItem* lOperand = Stack_pop(stack);
		Stack_push(stack, int_multiply(lOperand, rOperand));
	} else if (main->data[(*currentLocation)] == '/') {
		StackItem* rOperand = Stack_pop(stack);
		StackItem* lOperand = Stack_pop(stack);
		Stack_push(stack, int_divide(lOperand, rOperand));
	} else if (main->data[(*currentLocation)] == '(') {
		long label = stack->length;
		(*currentLocation) ++;
		while (main->data[(*currentLocation)] != ')') {
			exec(main, stack, localIndex, locals, localFrame, running, currentLocation, lineNumber);
			(*currentLocation) ++;
		}
		while (stack->length > label + 1) {
			StackItem* item = Stack_pop(stack);
			GC_free(item->item);
			GC_free(item);
		}
	} else if ((main->data[(*currentLocation)] >> 9) & 1) {
// Get Variable
		char* varName = (char*) &main->data[(*currentLocation) + 1];
		int length = (main->data[(*currentLocation)] ^ (1 << 9));
		(*currentLocation) += length;
		int found = 0;
		for (int i = 0 ; i < locals->length ; i ++) {
			LocalIndex* local = (LocalIndex*) ((StackItem*) locals->data[i])->item;
			if (strcmp(local->name, varName) == 0) {
				found = 1;
				Stack_push(stack, local->value);
				break;
			}
		}
		if (!found) {
			setError("variable used before assignment");
			running = 0;
		}
	} else if ((main->data[(*currentLocation)] >> 10) & 1) {
// Set Variable
		char* varName = (char*) &main->data[(*currentLocation) + 1];
		int length = (main->data[(*currentLocation)] ^ (1 << 10));
		(*currentLocation) += length;
		int found = 0;
		for (int i = 0 ; i < locals->length ; i ++) {
			LocalIndex* local = (LocalIndex*) ((StackItem*) locals->data[i])->item;
			if (strcmp(local->name, varName) == 0) {
				found = 1;
				local->value = Stack_pop(stack);
				break;
			}
		}
		if (!found) {
			LocalIndex* local = GC_MALLOC(sizeof(LocalIndex));
			local->name = varName;
			local->value = Stack_pop(stack);
			Stack_push(locals, newStackItem(Type_LOCAL_INDEX, local));
			if (debug) printf("var assigned: %s\n", StackItem_toString(locals->data[locals->length-1]));
		}
	} else if ((main->data[(*currentLocation)] >> 8) & 1) {
// Execute Function
		char* funcName = (char*) &main->data[(*currentLocation) + 1];
		int length = (main->data[(*currentLocation)] ^ (1 << 8));
		(*currentLocation) += length;
		//printf("function name is: %s\n", funcName);
		long label = stack->length;
		while (main->data[(*currentLocation)] != ')') {
			exec(main, stack, localIndex, locals, localFrame, running, currentLocation, lineNumber);
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
			int_printf(argStack);
			GC_free(argStack);
		} else if (strcmp(funcName, "dumpLocals") == 0) {
			dumpLocals(localIndex, locals, localFrame, stdout);
			while (stack->length > label) {
				StackItem* item = Stack_pop(stack);
				GC_free(item->item);
				GC_free(item);
			}
		} else if (strcmp(funcName, "dumpStack") == 0) {
			dumpStack(stack, stdout);
			while (stack->length > label) {
				StackItem* item = Stack_pop(stack);
				GC_free(item->item);
				GC_free(item);
			}
		}
	} else if ((main->data[(*currentLocation)] >> 7) & 1) {
// string constant
		char* val = (char*) &main->data[(*currentLocation) + 1];
		int length = (main->data[(*currentLocation)] ^ (1 << 7));
		/*
		for (int i = 0 ; i < length ; i ++) {
			printf("char: %d at: %d\n", val[i], i);
		}
		*/
		//printf("string: %s length: %d\n", val, length);
		(*currentLocation) += length;
		Stack_push(stack, newStackItem(Type_STRING, val));
	} else if ((main->data[(*currentLocation)] >> 11) & 255) {
		exit(0);
	}
}
// Interrupt Implementation
StackItem* int_add(StackItem* lOperand, StackItem* rOperand) {
	Type oType = rOperand->type > lOperand->type ? rOperand->type : lOperand->type;
	if (oType == Type_STRING) {
		fprintf(stderr, "strings types can't be used in math!!!\n");
		exit(1);
	}
	if (oType == Type_CHAR) {
		char* val = GC_MALLOC(sizeof(char));
		*val = (char) *lOperand->item + (char) *rOperand->item;
		return newStackItem(oType, val);
	} else if (oType == Type_INT) {
		int* val = GC_MALLOC(sizeof(int));
		*val = (int) *lOperand->item + (int) *rOperand->item;
		return newStackItem(oType, val);
	} else if (oType == Type_FLOAT) {
		float* val = GC_MALLOC(sizeof(float));
		fprintf(stderr, "casting int to float is not yet supported on this system!!!\n");
		exit(1);
		/*
		*val = 
			lOperand->type == Type_INT || lOperand->type == Type_CHAR ? (float) *((int*)lOperand->item) : (float) *((float*)lOperand->item)
			+ rOperand->type == Type_INT || rOperand->type == Type_CHAR ? (float) *((int*)rOperand->item) : (float) *((float*)rOperand->item);
		*/
		return newStackItem(oType, val);
	}
}
StackItem* int_subtract(StackItem* lOperand, StackItem* rOperand) {
	Type oType = rOperand->type > lOperand->type ? rOperand->type : lOperand->type;
	if (oType == Type_STRING) {
		fprintf(stderr, "strings types can't be used in math!!!\n");
		exit(1);
	}
	if (oType == Type_CHAR) {
		char* val = GC_MALLOC(sizeof(char));
		*val = (char) *lOperand->item - (char) *rOperand->item;
		return newStackItem(oType, val);
	} else if (oType == Type_INT) {
		int* val = GC_MALLOC(sizeof(int));
		*val = (int) *lOperand->item - (int) *rOperand->item;
		return newStackItem(oType, val);
	} else if (oType == Type_FLOAT) {
		float* val = GC_MALLOC(sizeof(float));
		fprintf(stderr, "casting int to float is not yet supported on this system!!!\n");
		exit(1);
		/*
		*val = 
			lOperand->type == Type_INT || lOperand->type == Type_CHAR ? (float) *((int*)lOperand->item) : (float) *((float*)lOperand->item)
			- rOperand->type == Type_INT || rOperand->type == Type_CHAR ? (float) *((int*)rOperand->item) : (float) *((float*)rOperand->item);
		*/
		return newStackItem(oType, val);
	}
}
StackItem* int_multiply(StackItem* lOperand, StackItem* rOperand) {
	Type oType = rOperand->type > lOperand->type ? rOperand->type : lOperand->type;
	if (oType == Type_STRING) {
		fprintf(stderr, "strings types can't be used in math!!!\n");
		exit(1);
	}
	if (oType == Type_CHAR) {
		char* val = GC_MALLOC(sizeof(char));
		*val = (char) *lOperand->item * (char) *rOperand->item;
		return newStackItem(oType, val);
	} else if (oType == Type_INT) {
		int* val = GC_MALLOC(sizeof(int));
		*val = (int) *lOperand->item * (int) *rOperand->item;
		return newStackItem(oType, val);
	} else if (oType == Type_FLOAT) {
		float* val = GC_MALLOC(sizeof(float));
		fprintf(stderr, "casting int to float is not yet supported on this system!!!\n");
		exit(1);
		/*
		*val = 
			lOperand->type == Type_INT || lOperand->type == Type_CHAR ? (float) *((int*)lOperand->item) : (float) *((float*)lOperand->item)
			* rOperand->type == Type_INT || rOperand->type == Type_CHAR ? (float) *((int*)rOperand->item) : (float) *((float*)rOperand->item);
		*/
		return newStackItem(oType, val);
	}
}
StackItem* int_divide(StackItem* lOperand, StackItem* rOperand) {
	Type oType = rOperand->type > lOperand->type ? rOperand->type : lOperand->type;
	if (oType == Type_STRING) {
		fprintf(stderr, "strings types can't be used in math!!!\n");
		exit(1);
	}
	if (oType == Type_CHAR) {
		char* val = GC_MALLOC(sizeof(char));
		*val = (char) *lOperand->item / (char) *rOperand->item;
		return newStackItem(oType, val);
	} else if (oType == Type_INT) {
		int* val = GC_MALLOC(sizeof(int));
		*val = (int) *lOperand->item / (int) *rOperand->item;
		return newStackItem(oType, val);
	} else if (oType == Type_FLOAT) {
		float* val = GC_MALLOC(sizeof(float));
		fprintf(stderr, "casting int to float is not yet supported on this system!!!\n");
		exit(1);
		/*
		*val = 
			lOperand->type == Type_INT || lOperand->type == Type_CHAR ? (float) *((int*)lOperand->item) : (float) *((float*)lOperand->item)
			/ rOperand->type == Type_INT || rOperand->type == Type_CHAR ? (float) *((int*)rOperand->item) : (float) *((float*)rOperand->item);
		*/
		return newStackItem(oType, val);
	}
}
void int_printf(Stack* argStack) {
	StackItem* format = Stack_pop(argStack);
	if (format->type == Type_STRING) {
		int start = 0, end = -1;
		char* fData = ((char*)format->item);
		int fLen = strlen(fData);
		int arg = 0;
		for (int i = 0 ; i <= fLen ; i ++) {
			if (fData[i] == '%' || i == fLen) {
				end = i;
				int len = end - start;
				char* temp = GC_MALLOC(sizeof(char) * (len + 1));
				for (int i = 0 ; i < len ; i ++) {
					temp[i] = fData[start + i];
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
				start = i;
			}
		}
	}
}
void setError(char* message) {
	if (debug) {
		printf("setting error message\n");
	}
	error = 1;
	errorMessage = GC_MALLOC(sizeof(char) * strlen(message));
	strcpy(errorMessage, message);
}
void dumpLocals(Stack* localIndex, Stack* locals, Stack* localFrame, FILE* stream) {
	fprintf(stream, "Locals:[");
	for (int i = 0 ; i < locals->length ; i ++) {
		StackItem* item = locals->data[i];
		fprintf(stream, "%s", StackItem_toString(item));
		if (i < locals->length - 1) {
			fprintf(stream, ", ");
		}
	}
	fprintf(stream, "]\n");
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
