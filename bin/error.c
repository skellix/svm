#include "error.h"

#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include "gc.h"

int error = 0;
char* errorMessage;
jmp_buf errBuf;

void throwException(char* message) {
	error = 1;
	errorMessage = realloc(errorMessage, (strlen(message) + 1) * sizeof(char));
	strcpy(errorMessage, message);
	longjmp(errBuf, 1);
}
