#include "stdfuncs.h"

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include "bytecode.h"
#include "stack.h"
#include "debug.h"
//#include "gc.h"
#include "io.h"
#include "error.h"
#include "interrupt.h"

int debug = 0;
int interactive = 0;
int breaksLen = 0;
long* breaks = NULL;
int frameDepth = 0;
void* debugLib;

typedef struct {
	char* name;
	void* impl;
} Func;

// These must be in alphabetical order
#define numFuncs 22
static Func funcList[] = {
	{"array",		int_array},
	{"break",		int_break},
	{"call",		int_call},
	{"class",		int_class},
	{"close",		int_close},
	{"continue",	int_continue},
	{"dumpFrame",	int_dumpFrame},
	{"dumpLocals",	int_dumpLocals},
	{"dumpStack",	int_dumpStack},
	{"function",	int_function},
	{"listClasses",	int_listClasses},
	{"listFunctions",int_listFunctions},
	{"new",			int_new},
	{"open",		int_open},
	{"print",		int_printf},
	{"read",		int_read},
	{"set",			int_set},
	{"size",		int_size},
	{"stackSize",	int_stackSize},
	{"stdin",		int_stdin},
	{"toString",	int_toString},
	{"write",		int_write},
};

static int cmpFuncs(const void* f1, const void* f2) {
	char* c1 = ((Func*) f1)->name;
	char* c2 = ((Func*) f2)->name;
	int i = 0;
	for (; c1[i] == c2[i] && c1[i] != 0 ; i ++);
	return c1[i] - c2[i];
}

void exec(Thread* thread) {
	if (debug > 0) {
		char c = thread->main->data[(*thread->currentLocation)];
		printf("%6lX| ", *thread->currentLocation);
		for (int i = frameDepth ; i > 0 ; i --) printf("  ");
		int (*_printOp)(short, Bytecode*, long*) = dlsym(debugLib, "printOp");
		if (dlerror()) {
			errorMessage = "unable to load libsvm.debug.so func printOp 1";
			longjmp(errBuf, 1);
		}
		_printOp((short)c, thread->main, thread->currentLocation);
	}
	//int call = thread->main->data[*thread->currentLocation];
	int opCall = thread->main->data[(*thread->currentLocation)];
	//printf("op:[%d]\n",  opCall);
	switch (opCall) {
		case FUNCTION: {
// Execute Function
			char* funcName = (char*) &thread->main->data[(*thread->currentLocation) + 5];
			int length = *(int*) &thread->main->data[(*thread->currentLocation) + 1];
			//printf("function:%s+%d\n", funcName, length);
			(*thread->currentLocation) += length + 1;
			long label = thread->stack->length;
			if (interactive) {
				int (*_debugCLI)(Bytecode*, long*, Stack*, Stack*, Stack*) = dlsym(debugLib, "debugCLI");
				if (dlerror() != NULL) {
					errorMessage = "unable to load libsvm.debug.so func debugCLI";
					longjmp(errBuf, 1);
				}
				while (thread->main->data[(*thread->currentLocation)] != ')') {
					while(_debugCLI(thread->main, thread->currentLocation, thread->stack, thread->locals, thread->localFrame));
					frameDepth ++;
					exec(thread);
					frameDepth --;
					(*thread->currentLocation) ++;
				}
			} else {
				while (thread->main->data[(*thread->currentLocation)] != ')') {
					exec(thread);
					(*thread->currentLocation) ++;
				}
			}
			if (debug > 0) {
				char c = thread->main->data[(*thread->currentLocation)];
				printf("%6lX| ", *thread->currentLocation);
				for (int i = frameDepth ; i > 0 ; i --) printf("  ");
				int (*_printOp)(short, Bytecode*, long*) = dlsym(debugLib, "printOp");
				if (dlerror() != NULL) {
					errorMessage = "unable to load libsvm.debug.so func printOp";
					longjmp(errBuf, 1);
				}
				_printOp(c, thread->main, thread->currentLocation);
			}
			if (funcName[0] == '#') {
				char* cfuncName = &funcName[1];
				printf("cfunc: %s\n", cfuncName);
				// TODO add this!
			} else {
				Func request = {funcName};
				Func* result = bsearch(&request, funcList, numFuncs, sizeof(Func), cmpFuncs);
				if (result != NULL) {
					void (*_funcCall)(Thread* thread, long label) = result->impl;
					_funcCall(thread, label);
				} else {
					char* msgPart = "function not found '";
					errorMessage = realloc(errorMessage, (strlen(msgPart) + strlen(funcName) + 2) * sizeof(char));
					strcat(errorMessage, msgPart);
					strcat(errorMessage, funcName);
					strcat(errorMessage, "'");
					longjmp(errBuf, 1);
				}
			}
			return;}
		case GET_VAR: {
// Get Variable
			char* varName = &thread->main->data[(*thread->currentLocation) + 5];
			int length = *(int*) &thread->main->data[(*thread->currentLocation) + 1];
			(*thread->currentLocation) += length;
			int found = 0;
			for (StackItem* item = thread->locals->data ; item->type != Type_NULL ; item = item->next) {
				LocalIndex* local = (LocalIndex*) item->item;
				if (strcmp(local->name, varName) == 0) {
					found = 1;
					StackItem* item = (StackItem*) local->value;
					if (item->type == Type_ARRAY) {
						Stack_push(thread->stack, newStackItem(Type_ARRAY_REF, item->item));
					} else if (item->type == Type_OBJECT) {
						Stack_push(thread->stack, newStackItem(Type_OBJECT_REF, item->item));
					} else if (item->type == Type_OBJECT_CHILD) {
						Stack_push(thread->stack, newStackItem(Type_OBJECT_CHILD_REF, item->item));
					} else {
						Stack_push(thread->stack, StackItem_clone(item));
					}
					return;
				}
			}
			if (!found) {
				throwException("variable used before assignment");
				*thread->running = 0;
			}
			return;}
		case STRING_CONST: {
// string constant
			int length = *(int*) &thread->main->data[(*thread->currentLocation) + 1];
			char* val = calloc(length - 5 + 1, sizeof(char));
			strcpy(val, &thread->main->data[(*thread->currentLocation) + 5]);
			//memcpy(val, (char*) &thread->main->data[(*thread->currentLocation) + 1], length * 2);
			(*thread->currentLocation) += length;
			Stack_push(thread->stack, newStackItem(Type_STRING, val));
			return;}
		case SET_VAR: {
// Set Variable
			char* varName = &thread->main->data[(*thread->currentLocation) + 5];
			int length = *(int*) &thread->main->data[(*thread->currentLocation) + 1];
			(*thread->currentLocation) += length;
			int found = 0;
			for (StackItem* item = thread->locals->data ; item->type != Type_NULL ; item = item->next) {
				LocalIndex* local = (LocalIndex*) item->item;
				if (strcmp(local->name, varName) == 0) {
					found = 1;
					StackItem_dispose(local->value);
					local->value = Stack_pop(thread->stack);
					break;
				}
			}
			if (!found) {
				StackItem* item = Stack_pop(thread->stack);
				LocalIndex* local = malloc(sizeof(LocalIndex));
				local->name = varName;
				local->value = item;
				Stack_push(thread->locals, newStackItem(Type_LOCAL_INDEX, local));
			}
			return;}
		case SYSTEM_EXIT: {
// System Exit
			Stack_dispose(thread->locals);
			Stack_dispose(thread->localFrame);
			Stack_dispose(thread->stack);
			//Stack_dispose(localIndex);
			Bytecode_dispose(thread->main);
			free(errorMessage);
			free(thread->running);
			free(thread->currentLocation);
			exit(0);
			return;}
		case INTEGER_CONST: {
// Integer Constant
			int* val = malloc(sizeof(int));
			//short length = thread->main->data[(*thread->currentLocation)] & (~(16 << 8));
			*val = *(int*) &thread->main->data[(*thread->currentLocation)+1];
			Stack_push(thread->stack, newStackItem(Type_INT, val));
			*thread->currentLocation += 4;
			return;}
		case LIGHT_FRAME: {
// Light Frame Begin
			long label = thread->stack->length;
			(*thread->currentLocation) += 5;
			while (thread->main->data[(*thread->currentLocation)] != ')') {
				exec(thread);
				(*thread->currentLocation) ++;
			}
			if (debug > 0) {
				int c = thread->main->data[(*thread->currentLocation)];
				printf("%6lX| ", *thread->currentLocation);
				int (*_printOp)(short, Bytecode*, long*) = dlsym(debugLib, "printOp");
				if (dlerror() != NULL) {
					errorMessage = "unable to load libsvm.debug.so func printOp";
					longjmp(errBuf, 1);
				}
				_printOp(c, thread->main, thread->currentLocation);
			}
			while (thread->stack->length > label + 1) {
				StackItem* item = Stack_pop(thread->stack);
				StackItem_dispose(item);
			}
			return;}
		case FRAME: {
// Frame Begin
			LocalFrame* frame = malloc(sizeof(LocalFrame));
			frame->start = *thread->currentLocation;
			int length = *(int*) &thread->main->data[(*thread->currentLocation) + 1];
			frame->length = length;
			frame->localCount = thread->locals->length;
			frame->stackSize = thread->stack->length;
			Stack_push(thread->localFrame, newStackItem(Type_LOCAL_FRAME, frame));
			*thread->currentLocation += 4;
			return;}
		default : {
			switch (opCall) {
				case '.': {
					StackItem* item = Stack_peek(thread->stack);
					if (item->type == Type_ARRAY) {
						Stack_push(thread->stack, newStackItem(Type_ARRAY_REF, item->item));
					} else {
						Stack_push(thread->stack, StackItem_clone(item));
					}
					return;}
				case 'x': {
					StackItem* item1 = Stack_pop(thread->stack);
					StackItem* item2 = Stack_pop(thread->stack);
					Stack_push(thread->stack, item1);
					Stack_push(thread->stack, item2);
					return;}
				case '+': {
					StackItem* rOperand = Stack_pop(thread->stack);
					StackItem* lOperand = Stack_pop(thread->stack);
					Stack_push(thread->stack, int_add(lOperand, rOperand));
					return;}
				case '-': {
					StackItem* rOperand = Stack_pop(thread->stack);
					StackItem* lOperand = Stack_pop(thread->stack);
					Stack_push(thread->stack, int_subtract(lOperand, rOperand));
					return;}
				case '*': {
					StackItem* rOperand = Stack_pop(thread->stack);
					StackItem* lOperand = Stack_pop(thread->stack);
					Stack_push(thread->stack, int_multiply(lOperand, rOperand));
					return;}
				case '/': {
					StackItem* rOperand = Stack_pop(thread->stack);
					StackItem* lOperand = Stack_pop(thread->stack);
					Stack_push(thread->stack, int_divide(lOperand, rOperand));
					return;}
				case '%': {
					StackItem* rOperand = Stack_pop(thread->stack);
					StackItem* lOperand = Stack_pop(thread->stack);
					Stack_push(thread->stack, int_mod(lOperand, rOperand));
					return;}
				case '<': {
					StackItem* rOperand = Stack_pop(thread->stack);
					StackItem* lOperand = Stack_pop(thread->stack);
					Stack_push(thread->stack, int_less(lOperand, rOperand));
					return;}
				case '>': {
					StackItem* rOperand = Stack_pop(thread->stack);
					StackItem* lOperand = Stack_pop(thread->stack);
					Stack_push(thread->stack, int_greater(lOperand, rOperand));
					return;}
				case '=': {
					StackItem* rOperand = Stack_pop(thread->stack);
					StackItem* lOperand = Stack_pop(thread->stack);
					Stack_push(thread->stack, int_equal(lOperand, rOperand));
					return;}
				case '&': {
					StackItem* rOperand = Stack_pop(thread->stack);
					StackItem* lOperand = Stack_pop(thread->stack);
					Stack_push(thread->stack, int_and(lOperand, rOperand));
					return;}
				case '|': {
					StackItem* rOperand = Stack_pop(thread->stack);
					StackItem* lOperand = Stack_pop(thread->stack);
					Stack_push(thread->stack, int_or(lOperand, rOperand));
					return;}
				case '^': {
					StackItem* rOperand = Stack_pop(thread->stack);
					StackItem* lOperand = Stack_pop(thread->stack);
					Stack_push(thread->stack, int_xor(lOperand, rOperand));
					return;}
				case '~': {
					StackItem* item = Stack_pop(thread->stack);
					Stack_push(thread->stack, int_bit_not(item));
					return;}
				case '!': {
					StackItem* item = Stack_pop(thread->stack);
					Stack_push(thread->stack, int_logic_not(item));
					return;}
				case '?': {
					StackItem* item = Stack_pop(thread->stack);
					if (thread->main->data[(*thread->currentLocation) + 1] == FRAME
							|| thread->main->data[(*thread->currentLocation) + 1] == LIGHT_FRAME) {
						int length = *(int*) &thread->main->data[(*thread->currentLocation) + 2];
						if (item->type == Type_CHAR && *(char*) item->item == 0) {
							*thread->currentLocation += length + 1;
						} else if (item->type == Type_INT && *(int*) item->item == 0) {
							*thread->currentLocation += length + 1;
						} else if (item->type == Type_FLOAT && *(float*) item->item == 0) {
							*thread->currentLocation += length + 1;
						}
					}
					StackItem_dispose(item);
					return;}
				case '}': {
// Frame End
					StackItem* frameItem = Stack_pop(thread->localFrame);
					LocalFrame* frame = (LocalFrame*) frameItem->item;
					while (thread->stack->length > frame->stackSize + 1) {
						StackItem* item = Stack_pop(thread->stack);
						StackItem_dispose(item);
					}
					while (thread->locals->length > frame->localCount) {
						StackItem* item = Stack_pop(thread->locals);
						StackItem_dispose(item);
					}
					StackItem_dispose(frameItem);
					return;}
				case '[': {
					StackItem* arrayItem = Stack_pop(thread->stack);
					if (arrayItem->type == Type_ARRAY || arrayItem->type == Type_ARRAY_REF) {
						Array* array = arrayItem->item;
						long label = thread->stack->length;
						(*thread->currentLocation) ++;
						while (thread->main->data[(*thread->currentLocation)] != ']') {
							exec(thread);
							(*thread->currentLocation) ++;
						}
						if (debug > 0) {
							int c = thread->main->data[(*thread->currentLocation)];
							printf("%6lX| ", *thread->currentLocation);
							int (*_printOp)(short, Bytecode*, long*) = dlsym(debugLib, "printOp");
							if (dlerror() != NULL) {
								errorMessage = "unable to load libsvm.debug.so func printOp";
								longjmp(errBuf, 1);
							}
							_printOp(c, thread->main, thread->currentLocation);
						}
						while (thread->stack->length > label + 1) {
							StackItem* item = Stack_pop(thread->stack);
							StackItem_dispose(item);
						}
						StackItem* indexItem = Stack_pop(thread->stack);
						if (indexItem->type == Type_INT) {
							int index = *(int*) indexItem->item;
							if (index < array->length) {
								Stack_push(thread->stack, StackItem_clone(&array->data[index]));
							} else {
								char* msg = "array index out of bounds: %d ";
								int length = 11;
								errorMessage = realloc(errorMessage, (strlen(msg) + length + 1) * sizeof(char));
								memset(errorMessage, 0, strlen(msg) + length + 1);
								sprintf(errorMessage, msg, index);
								longjmp(errBuf, 1);
							}
						}
					} else {
						free(errorMessage);
						errorMessage = "brackets can only be used to access array items";
						longjmp(errBuf, 1);
					}
					StackItem_dispose(arrayItem);
					return;}
			}
			return;}
	}
}



void Bytecode_dispose(Bytecode* code) {
	free(code->data);
	free(code->lineNumbers);
	free(code->name);
	free(code);
}

int runBytecode(Thread* thread) {
	*thread->running = 1;
	if (debug > 0) {
		printf("\n------.\n");
	}
	if (interactive) {
		printf("Source Virtual Machine Debugger\n");
		printf("\tVersion: 0.0.0\n");
		printf("\tType '?' for help\n");
		breaksLen = 0;
		breaks = malloc(sizeof(long) * (++ breaksLen));
		breaks[0] = 0L;
		int (*_debugCLI)(Bytecode*, long*, Stack*, Stack*, Stack*) = dlsym(debugLib, "debugCLI");
		if (dlerror() != NULL) {
			errorMessage = "unable to load libsvm.debug.so func debugCLI";
			longjmp(errBuf, 1);
		}
		while (*thread->running) {
			while (_debugCLI(thread->main, thread->currentLocation, thread->stack, thread->locals, thread->localFrame));
			exec(thread);
			(*thread->currentLocation) ++;
			if (*thread->currentLocation > thread->main->length) {
				if (thread->stack->length > 0) {
					StackItem* item = Stack_pop(thread->stack);
					if (item->type == Type_INT) {
						return *(int*) item->item;
					}
				}
			}
		}
	} else {
		while (*thread->running) {
			exec(thread);
			(*thread->currentLocation) ++;
			if (*thread->currentLocation > thread->main->length) {
				if (thread->stack->length > 0) {
					StackItem* item = Stack_pop(thread->stack);
					if (item->type == Type_INT) {
						return *(int*) item->item;
					}
				}
			}
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

void dumpFrame(Stack* locals, Stack* localFrame, FILE* stream) {
	fprintf(stream, "Frames:[");
	for (StackItem* item = localFrame->data ; item->type != Type_NULL ; item = item->next) {
		char* str = StackItem_toString(item);
		fprintf(stream, "%s", str);
		free(str);
		if (item->next->type != Type_NULL) {
			fprintf(stream, ", ");
		}
	}
	fprintf(stream, "]\n");
}

void dumpLocals(Stack* locals, Stack* localFrame, FILE* stream) {
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
