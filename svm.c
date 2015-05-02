/*
 * Source Virtual Machine
 *
 * "Assembly + Lisp"
 *
 */
#include <stdio.h>
#include <setjmp.h>
#include <stdarg.h>
#include <string.h>
#include "gc.h"
#include "bytecode.h"
#include "stack.h"
#include "error.h"
#include "io.h"
#include "debug.h"
#include "interrupt.h"
#include "opcodes.h"
// INDEX OF FUNCTIONS
//   VM Functions
int runBytecode(Bytecode* main, long* currentLocation, Stack* stack, Stack* localIndex, Stack* locals, Stack* localFrame);
void exec(Bytecode* main, Stack* stack, Stack* localIndex, Stack* locals, Stack* localFrame, int* running, long* currentLocation);

void showHelp(){
	fprintf(stderr, "USAGE: foo <input>\n");
}

int main(int argc, char** argv) {
	if (argc <= 1) {
		showHelp(); return 0;
	}
	char* srcName = NULL;
	int norun = 0;
	int output = 0;
	char* outputName = NULL;
	int argStart = -1;
	for (int i = 1 ; i < argc ; i ++) {
		if (argv[i][0] == '-') {
			for (char* c = argv[i] + 1 ; *c != 0 ; c ++) {
				if (*c == 'X') debug ++;
				else if (*c == 'E') norun = 1;
				else if (*c == 'g') interactive = 1;
				else if (*c == 'o') {
					output = 1;
					outputName = argv[++ i];
				}
			}
		} else if (srcName == NULL) {
			srcName = argv[i];
		} else if (argStart == -1) {
			argStart = i;
		}
	}
	//GC_init();
	//GC_find_leak = 1;
	errorMessage = "";
	Bytecode* main = Bytecode_loadFromFile(srcName, debug);
	if (debug) {
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
                length = printOp(c, main, &i);
                indent ++;
			} else if ((char) c == ')' || (char) c == '}') {
			    indent --;
                for (int j = indent ; j > 0 ; j --) printf("  ");
                printf("%c\n", (char) c);
			} else {
			    for (int j = indent ; j > 0 ; j --) printf("  ");
                length = printOp(c, main, &i);
			}
			if (debug > 3) {
				for (int j = i + 1 ; j <= i + length ; j ++) {
					short c = main->data[j];
					for (int j = 15 ; j >= 0 ; j --) {
						printf("%c", (((c >> j) & 1) == 1) ? '1' : '0');
						if (j % 4 == 0) printf(" ");
					}
					printf("| %.4X | %5lX\n", (int) c & 0xFFFF, j);
				}
			}
			i += length;
		}
		printf("--------------------'------'------'\n");
		printf("\n------.\n");
	}
	if (!norun) {
		Stack* stack = Stack_create();
		Stack* localIndex = Stack_create();
		Stack* locals = Stack_create();
		Stack* localFrame = Stack_create();
// Push program arguments onto the stack
		if (argStart > 0) {
			for (int i = argc - 1 ; i >= argStart ; i --) {
				char* val = calloc(strlen(argv[i]) + 1, sizeof(char));
				strcpy(val, argv[i]);
				Stack_push(stack, newStackItem(Type_STRING, val));
			}
		}
// Create main frame
		LocalFrame* frame = malloc(sizeof(LocalFrame));
		//printf("creating frame: %10lX\n", frame);
		frame->start = 0;
		frame->length = main->length;
		frame->localCount = 0;
		frame->stackSize = 0;
		Stack_push(localFrame, newStackItem(Type_LOCAL_FRAME, frame));
//Start main execution loop
		long currentLocation = 0;
		int exitStatus = -1;
		if (setjmp(errBuf)) {
			int line = 1;
			for (int i = 0 ; i < main->lines ; i ++) {
				if (main->lineNumbers[i] > currentLocation) {
					line = i + 1;
					break;
				}
			}
			//if (debug > 2) GC_gcollect();
			fprintf(stderr, "Error: %s at line %d:\n", errorMessage, line);
			dumpLocals(localIndex, locals, localFrame, stderr);
			dumpStack(stack, stderr);
			return 1;
		}
		if ((exitStatus = runBytecode(main, &currentLocation, stack, localIndex, locals, localFrame)) != 0) {
			if (error) {
				fprintf(stderr, "Error %s at line %d:\n", errorMessage, -1);
			} else {
				fprintf(stderr, "Error %d at line %d:\n", exitStatus, -1);
			}
			dumpLocals(localIndex, locals, localFrame, stderr);
			dumpStack(stack, stderr);
			return 1;
		}
		Bytecode_dispose(main);
	}
	return 0;
}

int runBytecode(Bytecode* main, long* currentLocation, Stack* stack, Stack* localIndex, Stack* locals, Stack* localFrame) {
	int running = 1;
	if (interactive) {
		printf("Source Virtual Machine Debugger\n");
		printf("\tVersion: 0.0.0\n");
		printf("\tType '?' for help\n");
		breaksLen = 0;
		breaks = malloc(sizeof(long) * (++ breaksLen));
		breaks[0] = 0L;
		while (running) {
			while (debugCLI(main, currentLocation, stack, localIndex, locals, localFrame));
			exec(main, stack, localIndex, locals, localFrame, &running, currentLocation);
			(*currentLocation) ++;
			if (*currentLocation > main->length) {
				if (stack->length > 0) {
					StackItem* item = Stack_pop(stack);
					if (item->type == Type_INT) {
						return *(int*) item->item;
					}
				}
			}
			//if (debug > 2) GC_gcollect();
		}
	} else {
		while (running) {
			exec(main, stack, localIndex, locals, localFrame, &running, currentLocation);
			(*currentLocation) ++;
			if (*currentLocation > main->length) {
				if (stack->length > 0) {
					StackItem* item = Stack_pop(stack);
					if (item->type == Type_INT) {
						return *(int*) item->item;
					}
				}
			}
			//if (debug > 2) GC_gcollect();
		}
	}
	return 0;
}

void exec(Bytecode* main, Stack* stack, Stack* localIndex, Stack* locals, Stack* localFrame, int* running, long* currentLocation) {
	if (debug > 1) {
		short c = main->data[(*currentLocation)];
		printf("%6lX| ", *currentLocation);
		for (int i = frameDepth ; i > 0 ; i --) printf("  ");
		printOp(c, main, currentLocation);
		//printf("command: '%c'\n", main->data[(*currentLocation)]);
	}
	int call = main->data[(*currentLocation)];
	int opCall = (main->data[(*currentLocation)] >> 8) & 0xFF;
	switch (opCall) {
		case FUNCTION: {
// Execute Function
			char* funcName = (char*) &main->data[(*currentLocation) + 1];
			int length = (main->data[(*currentLocation)] ^ (1 << 8));
			(*currentLocation) += length + 1;
			long label = stack->length;
			while (main->data[(*currentLocation)] != ')') {
				if (interactive) {
					while(debugCLI(main, currentLocation, stack, localIndex, locals, localFrame));
				}
				frameDepth ++;
				exec(main, stack, localIndex, locals, localFrame, running, currentLocation);
				frameDepth --;
				(*currentLocation) ++;
			}
			if (debug > 1) {
				short c = main->data[(*currentLocation)];
				printf("%6lX| ", *currentLocation);
				for (int i = frameDepth ; i > 0 ; i --) printf("  ");
				printOp(c, main, currentLocation);
			}
			//if (debug) {
			//	printf("called function '%s'\n", funcName);
			//}
			int cleanUp = 0;
			if (strcmp(funcName, "print") == 0) {
				Stack* argStack = Stack_create();
				while (stack->length > label) {
					Stack_push(argStack, Stack_pop(stack));
				}
				int_printf(argStack);
				Stack_dispose(argStack);
			} else if (strcmp(funcName, "continue") == 0) {
				StackItem* frameItem = Stack_peek(localFrame);
				LocalFrame* frame = (LocalFrame*) frameItem->item;
				while (stack->length > frame->stackSize + 1) {
					StackItem* item = Stack_pop(stack);
					StackItem_dispose(item);
				}
				while (locals->length > frame->localCount) {
					StackItem* item = Stack_pop(locals);
					StackItem_dispose(item);
				}
				*currentLocation = frame->start;
			} else if (strcmp(funcName, "break") == 0) {
				StackItem* frameItem = Stack_peek(localFrame);
				LocalFrame* frame = (LocalFrame*) frameItem->item;
				*currentLocation = frame->start + frame->length;
			} else if (strcmp(funcName, "open") == 0) {
				StackItem* mode = Stack_pop(stack);
				StackItem* address = Stack_pop(stack);
				Stack_push(stack, openStream(address->item, mode->item));
				StackItem_dispose(address);
				StackItem_dispose(mode);
			} else if (strcmp(funcName, "read") == 0) {
				Stack* argStack = Stack_create();
				while (stack->length > label) {
					Stack_push(argStack, Stack_pop(stack));
				}
				if (argStack->length > 1) {
					StackItem* stream = Stack_pop(argStack);
					StackItem* amt = Stack_pop(argStack);
					if (stream->type == Type_STREAM) {
						Stack_push(stack, readStream(*(int*) stream->item, *(int*) amt->item));
					} else if (stream->type == Type_FILE_STREAM) {
						Stack_push(stack, readFileStream((FILE*) stream->item, *(int*) amt->item));
					}
					StackItem_dispose(stream);
					StackItem_dispose(amt);
				} else {
					StackItem* format = Stack_pop(argStack);
					int start = 0, end = -1;
					char* fData = ((char*)format->item);
					int fLen = strlen(fData);
					int arg = 0;
					for (int i = 0 ; i <= fLen ; i ++) {
						if (fData[i] == '%' || i == fLen) {
							end = i;
							int len = end - start;
							char* temp = calloc(len + 1, sizeof(char));
							for (int i = 0 ; i < len ; i ++) {
								temp[i] = fData[start + i];
							}
							//snprintf(fData + start1, len, "%s", temp);
							temp[len] = 0;
							if (arg) {
								char* val = calloc(255, sizeof(char));
								scanf(format->item, val);
								val = realloc(val, (strlen(val) + 1) * sizeof(char));
								Stack_push(stack, newStackItem(Type_STRING, val));
							} else {
								printf(temp);
								arg = 1;
							}
							free(temp);
							start = i;
						}
					}
				}
				Stack_dispose(argStack);
			} else if (strcmp(funcName, "close") == 0) {
				StackItem* stream = Stack_pop(stack);
				closeStream(stream);
				StackItem_dispose(stream);
			} else if (strcmp(funcName, "strlen") == 0) {
				StackItem* item = Stack_pop(stack);
				int* val = malloc(sizeof(int));
				*val = strlen((char*) item->item);
				Stack_push(stack, newStackItem(Type_INT, val));
				StackItem_dispose(item);
			} else if (strcmp(funcName, "stackSize") == 0) {
				int* val = malloc(sizeof(int));
				*val = stack->length;
				Stack_push(stack, newStackItem(Type_INT, val));
			} else if (strcmp(funcName, "dumpLocals") == 0) {
				dumpLocals(localIndex, locals, localFrame, stdout);
				cleanUp = 1;
			} else if (strcmp(funcName, "dumpFrame") == 0) {
				dumpFrame(localIndex, locals, localFrame, stdout);
				cleanUp = 1;
			} else if (strcmp(funcName, "dumpStack") == 0) {
				dumpStack(stack, stdout);
				cleanUp = 1;
			} else if (funcName[0] == '#') {
				char* cfuncName = &funcName[1];
				printf("cfunc: %s\n", cfuncName);
				// TODO add this!
			}
			if (cleanUp) {
				while (stack->length > label) {
					StackItem* item = Stack_pop(stack);
					StackItem_dispose(item);
				}
			}
			return;}
		case GET_VAR: {
// Get Variable
			char* varName = (char*) &main->data[(*currentLocation) + 1];
			int length = (main->data[(*currentLocation)] ^ (2 << 8));
			(*currentLocation) += length;
			int found = 0;
			for (StackItem* item = locals->data ; item->type != Type_NULL ; item = item->next) {
				LocalIndex* local = (LocalIndex*) item->item;
				if (strcmp(local->name, varName) == 0) {
					found = 1;
					StackItem* item = (StackItem*) local->value;
					Stack_push(stack, StackItem_clone(item));
					return;
				}
			}
			if (!found) {
				throwException("variable used before assignment");
				running = 0;
			}
			return;}
		case STRING_CONST: {
// string constant
			int length = (main->data[(*currentLocation)] & (~(3 << 8)));
			char* val = calloc((length * 2) + 1, sizeof(char));
			memcpy(val, (char*) &main->data[(*currentLocation) + 1], length * 2);
			(*currentLocation) += length;
			Stack_push(stack, newStackItem(Type_STRING, val));
			return;}
		case SET_VAR: {
// Set Variable
			char* varName = (char*) &main->data[(*currentLocation) + 1];
			int length = (main->data[(*currentLocation)] & (~(4 << 8)));
			(*currentLocation) += length;
			int found = 0;
			for (StackItem* item = locals->data ; item->type != Type_NULL ; item = item->next) {
				LocalIndex* local = (LocalIndex*) item->item;
				if (strcmp(local->name, varName) == 0) {
					found = 1;
					StackItem_dispose(local->value);
					local->value = Stack_pop(stack);
					//if (debug > 2) {
					//	printf("stackItem at %p\n", (locals->data[locals->length-1]));
					//	printf("var: %s at %p\n", varName, (local));
					//	printf("varValue at %p\n", (local->value));
					//}
					//if (debug) {
					//	char* str = StackItem_toString(locals->data);
					//	printf("var assigned: %s\n", str);
					//	free(str);
					//}
					break;
				}
			}
			if (!found) {
				StackItem* item = Stack_pop(stack);
				LocalIndex* local = malloc(sizeof(LocalIndex));
				//printf("creating index: %10lX\n", local);
				local->name = varName;
				local->value = item;
				Stack_push(locals, newStackItem(Type_LOCAL_INDEX, local));
				//if (debug > 2) {
					//printf("stackItem at %p\n", (locals->data[locals->length-1]));
					//printf("var: %s at %p\n", varName, (local));
					//printf("varValue at %p\n", (local->value));
				//}
				//if (debug) {
				//	dumpLocals(localIndex, locals, localFrame, stdout);
				//}
			}
			return;}
		case SYSTEM_EXIT: {
// System Exit
			//dumpLocals(localIndex, locals, localFrame, stdout);
			Stack_dispose(locals);
			//dumpFrame(localIndex, locals, localFrame, stdout);
			Stack_dispose(localFrame);
			//dumpStack(stack, stdout);
			Stack_dispose(stack);
			Stack_dispose(localIndex);
			*running = 0;
			//exit(0);
			return;}
		case INTEGER_CONST: {
// Integer Constant
			int* val = malloc(sizeof(int));
			short length = main->data[(*currentLocation)] & (~(16 << 8));
			*val = *(int*) &main->data[(*currentLocation)+1];
			Stack_push(stack, newStackItem(Type_INT, val));
			*currentLocation += length;
			return;}
		case LIGHT_FRAME: {
// Light Frame Begin
			long label = stack->length;
			(*currentLocation) ++;
			while (main->data[(*currentLocation)] != ')') {
				exec(main, stack, localIndex, locals, localFrame, running, currentLocation);
				(*currentLocation) ++;
			}
			if (debug > 1) {
				short c = main->data[(*currentLocation)];
				printf("%6lX| ", *currentLocation);
				printOp(c, main, currentLocation);
				//printf("command: '%c'\n", main->data[(*currentLocation)]);
			}
			while (stack->length > label + 1) {
				StackItem* item = Stack_pop(stack);
				StackItem_dispose(item);
			}
			return;}
		case FRAME: {
// Frame Begin
			LocalFrame* frame = malloc(sizeof(LocalFrame));
			//printf("creating frame: %10lX\n", frame);
			frame->start = *currentLocation;
			short length = main->data[(*currentLocation)] & (~(128 << 8));
			frame->length = length;
			frame->localCount = locals->length;
			frame->stackSize = stack->length;
			Stack_push(localFrame, newStackItem(Type_LOCAL_FRAME, frame));
			return;}
		case OTHER: {
			switch (call) {
				case '.': {
					StackItem* item = Stack_peek(stack);
					Stack_push(stack, StackItem_clone(item));
					return;}
				case 'x': {
					StackItem* item1 = Stack_pop(stack);
					StackItem* item2 = Stack_pop(stack);
					Stack_push(stack, item1);
					Stack_push(stack, item2);
					return;}
				case '+': {
					StackItem* rOperand = Stack_pop(stack);
					StackItem* lOperand = Stack_pop(stack);
					Stack_push(stack, int_add(lOperand, rOperand));
					return;}
				case '-': {
					StackItem* rOperand = Stack_pop(stack);
					StackItem* lOperand = Stack_pop(stack);
					Stack_push(stack, int_subtract(lOperand, rOperand));
					return;}
				case '*': {
					StackItem* rOperand = Stack_pop(stack);
					StackItem* lOperand = Stack_pop(stack);
					Stack_push(stack, int_multiply(lOperand, rOperand));
					return;}
				case '/': {
					StackItem* rOperand = Stack_pop(stack);
					StackItem* lOperand = Stack_pop(stack);
					Stack_push(stack, int_divide(lOperand, rOperand));
					return;}
				case '%': {
					StackItem* rOperand = Stack_pop(stack);
					StackItem* lOperand = Stack_pop(stack);
					Stack_push(stack, int_mod(lOperand, rOperand));
					return;}
				case '<': {
					StackItem* rOperand = Stack_pop(stack);
					StackItem* lOperand = Stack_pop(stack);
					Stack_push(stack, int_less(lOperand, rOperand));
					return;}
				case '>': {
					StackItem* rOperand = Stack_pop(stack);
					StackItem* lOperand = Stack_pop(stack);
					Stack_push(stack, int_greater(lOperand, rOperand));
					return;}
				case '=': {
					StackItem* rOperand = Stack_pop(stack);
					StackItem* lOperand = Stack_pop(stack);
					Stack_push(stack, int_equal(lOperand, rOperand));
					return;}
				case '&': {
					StackItem* rOperand = Stack_pop(stack);
					StackItem* lOperand = Stack_pop(stack);
					Stack_push(stack, int_and(lOperand, rOperand));
					return;}
				case '|': {
					StackItem* rOperand = Stack_pop(stack);
					StackItem* lOperand = Stack_pop(stack);
					Stack_push(stack, int_or(lOperand, rOperand));
					return;}
				case '^': {
					StackItem* rOperand = Stack_pop(stack);
					StackItem* lOperand = Stack_pop(stack);
					Stack_push(stack, int_xor(lOperand, rOperand));
					return;}
				case '~': {
					StackItem* item = Stack_pop(stack);
					Stack_push(stack, int_bit_not(item));
					return;}
				case '!': {
					StackItem* item = Stack_pop(stack);
					Stack_push(stack, int_logic_not(item));
					return;}
				case '?': {
					StackItem* item = Stack_pop(stack);
					if ((main->data[(*currentLocation) + 1] >> 15) & 1) {
						if (item->type == Type_CHAR && *((char*)item->item) == 0) {
							short jump = main->data[(*currentLocation) + 1] & (~(128 << 8));
							*currentLocation += jump;
						} else if (item->type == Type_INT && *((int*)item->item) == 0) {
							short jump = main->data[(*currentLocation) + 1] & (~(128 << 8));
							*currentLocation += jump;
						} else if (item->type == Type_FLOAT && *((float*)item->item) == 0) {
							short jump = main->data[(*currentLocation) + 1] & (~(128 << 8));
							*currentLocation += jump;
						}
					} else {
						if (item->type == Type_CHAR && *((char*)item->item) == 0) {
							short jump = main->data[(*currentLocation) + 1] & (~(64 << 8));
							*currentLocation += jump;
						} else if (item->type == Type_INT && *((int*)item->item) == 0) {
							short jump = main->data[(*currentLocation) + 1] & (~(64 << 8));
							*currentLocation += jump;
						} else if (item->type == Type_FLOAT && *((float*)item->item) == 0) {
							short jump = main->data[(*currentLocation) + 1] & (~(64 << 8));
							*currentLocation += jump;
						}
					}
					StackItem_dispose(item);
					return;}
				case '}': {
// Frame End
					StackItem* frameItem = Stack_pop(localFrame);
					LocalFrame* frame = (LocalFrame*) frameItem->item;
					while (stack->length > frame->stackSize + 1) {
						StackItem* item = Stack_pop(stack);
						StackItem_dispose(item);
					}
					while (locals->length > frame->localCount) {
						StackItem* item = Stack_pop(locals);
						StackItem_dispose(item);
					}
					StackItem_dispose(frameItem);
					return;}
			}
			return;}
	}
}
