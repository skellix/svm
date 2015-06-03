#ifndef ERROR_H
#define ERROR_H

#include <setjmp.h>
#include "gc.h"

extern int error;
extern char* errorMessage;
extern jmp_buf errBuf;

void throwException(char* message);

#endif
