/*
 * Source Virtual Machine
 *
 * "Assembly + Lisp"
 *
 */
#include <stdio.h>
#include <setjmp.h>
#include <string.h>
#include <dlfcn.h>
//#include "gc.h"
#include "bytecode.h"
#include "stack.h"
#include "error.h"
#include "debug.h"
#include "stdfuncs.h"

void showHelp(){
	fprintf(stderr, "USAGE: svm [-(g|o|X|E)] <input>\n");
}

int main(int argc, char** argv) {
	if (argc <= 1) {
		showHelp(); return 0;
	}
	char* srcName = NULL;
	int norun = 0;
	//int output = 0;
	//char* outputName = NULL;
	int argStart = -1;
	for (int i = 1 ; i < argc ; i ++) {
		if (argv[i][0] == '-') {
			for (char* c = argv[i] + 1 ; *c != 0 ; c ++) {
				if (*c == 'X') debug ++;
				else if (*c == 'E') norun = 1;
				else if (*c == 'g') interactive = 1;
				//else if (*c == 'o') {
				//	output = 1;
				//	outputName = argv[++ i];
				//}
			}
		} else if (srcName == NULL) {
			srcName = argv[i];
		} else if (argStart == -1) {
			argStart = i;
		}
	}
	if (debug) {
		debugLib = dlopen("libsvm.debug.so", RTLD_NOW);
		if (dlerror() != NULL) {
			fprintf(stderr, "unable to find libsvm.debug.so");
			return 1;
		}
	}
	Bytecode* main;
	if (1) {// Call the loader
		errorMessage = calloc(0, sizeof(char));
		void* bytecode = dlopen("libsvm.bytecode.so", RTLD_LAZY);
		if (!bytecode) {
			printf("unable to load bytecode!!!\n");
			return 1;
		}
		dlerror();
		Bytecode* (*_load)(char* fileName, int debug) = dlsym(bytecode, "Bytecode_loadFromFile");
		if (dlerror()) {
			printf("unable to load bytecode func!!!\n");
			return 1;
		}
		main = _load(srcName, debug);
		dlclose(bytecode);
	}
	if (!norun) {
		Thread* mainThread = newThread(main);
// Push program arguments onto the stack
		if (argStart > 0) {
			for (int i = argc - 1 ; i >= argStart ; i --) {
				char* val = calloc(strlen(argv[i]) + 1, sizeof(char));
				strcpy(val, argv[i]);
				Stack_push(mainThread->stack, newStackItem(Type_STRING, val));
			}
		}
// Create main frame
		LocalFrame* frame = calloc(1, sizeof(LocalFrame));
		frame->length = main->length;
		Stack_push(mainThread->localFrame, newStackItem(Type_LOCAL_FRAME, frame));
//Start main execution loop
		int exitStatus = -1;
		if (setjmp(errBuf)) {
			int line = 1;
			for (int i = 0 ; i < main->lines ; i ++) {
				if (main->lineNumbers[i] > *mainThread->currentLocation) {
					line = i + 1;
					break;
				}
			}
			fprintf(stderr, "Error: %s at line %d:\n", errorMessage, line);
			fprintf(stderr, "index: %X\n", *mainThread->currentLocation);
			dumpLocals(mainThread->locals, mainThread->localFrame, stderr);
			dumpStack(mainThread->stack, stderr);
			return 1;
		}
		if ((exitStatus = runBytecode(mainThread)) != 0) {
			fprintf(stderr, "Process returned a non zero exit value: %s\n", exitStatus);
			dumpLocals(mainThread->locals, mainThread->localFrame, stderr);
			dumpStack(mainThread->stack, stderr);
			return 1;
		}
	}
	return 0;
}
