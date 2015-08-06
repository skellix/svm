#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <setjmp.h>
#include <stdarg.h>
#include <string.h>
//#include "gc.h"
#include "bytecode.h"
#include "stack.h"
#include "error.h"
#include "io.h"
#include "opcodes.h"

// Debug Headers
int printOp(short c, Bytecode* main, long* currentLocation);
int opLength(short c, Bytecode* main, long* currentLocation);
int debugCLI(Bytecode* main, long* currentLocation, Stack* stack, Stack* locals, Stack* localFrame);

#endif//DEBUG_H
