#ifndef STDFUNCS_H
#define STDFUNCS_H

#include "bytecode.h"
#include "stack.h"
#include "debug.h"
#include "gc.h"
#include "io.h"
#include "error.h"

void exec(Thread* thread);
void stdFunc(char* funcName, Thread* thread);

void Bytecode_dispose(Bytecode* code);
int runBytecode(Thread* thread);

void dumpStack(Stack* stack, FILE* stream);
void dumpFrame(Stack* locals, Stack* localFrame, FILE* stream);
void dumpLocals(Stack* locals, Stack* localFrame, FILE* stream);

// Debug vars included here too
//  until I find a better place for them
extern int debug;
extern int interactive;
extern int breaksLen;
extern long* breaks;
extern int frameDepth;
extern void* debugLib;

#endif//STDFUNCS_H
