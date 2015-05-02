#ifndef ERROR_H
#define ERROR_H

#include <setjmp.h>
#include "gc.h"

int error = 0;
char* errorMessage = NULL;
static jmp_buf errBuf;

void throwException(char* message) {
	error = 1;
	errorMessage = malloc(sizeof(char) * strlen(message));
	strcpy(errorMessage, message);
	longjmp(errBuf, 1);
}

#endif
