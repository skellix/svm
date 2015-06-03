#ifndef STACK_H
#define STACK_H

#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <stdio.h>
//#include <math.h>
#include "gc.h"
#include "error.h"
#include "bytecode.h"


typedef enum {
	Type_CHAR,
	Type_INT,
	Type_FLOAT,
	Type_STRING,
	Type_ARRAY,
	Type_ARRAY_REF,
	Type_STACK,
	Type_STACK_REF,
	Type_STREAM,
	Type_FILE_STREAM,
	Type_LOCAL_INDEX,
	Type_LOCAL_FRAME,
	Type_FUNCTION,
	Type_OBJECT,
	Type_OBJECT_REF,
	Type_OBJECT_CHILD,
	Type_OBJECT_CHILD_REF,
	Type_NULL,
} Type;

typedef struct StackItem_ {
	Type type;
	struct StackItem_* next;
	void* item;
} StackItem;

typedef struct {
	char* name;
	StackItem* value;
} LocalIndex;

typedef struct {
	int start, length;
	int localCount;
	int stackSize;
} LocalFrame;

typedef struct {
	int length;
	StackItem* data;
} Stack;

typedef struct {
	int length;
	StackItem* data;
} Array;

typedef struct {
	Bytecode* main;
	Stack* stack;
	Stack* locals;
	Stack* localFrame;
	Stack* functions;
	Stack* classes;
	int* running;
	long* currentLocation;
} Thread;

typedef struct {
	char* name;
	long address;
} Function;

typedef struct {
	char* name;
	int numVars;
	StackItem* vars;
	int numFunctions;
	Function* functions;
} Object;

void StackItem_dispose(StackItem* item);
void LocalIndex_dispose(LocalIndex* index);
void LocalFrame_dispose(LocalFrame* frame);
void Array_dispose(Array* array);

char* typeName(Type type);

// String
char* stringValue(StackItem* item);
char* StackItem_toString(StackItem* item);
char* LocalIndex_toString(LocalIndex* item);
char* LocalFrame_toString(LocalFrame* item);

// Value
StackItem* newStackItem(Type type, void* item);
Array* newArray();
StackItem* StackItem_clone(StackItem* item);
void* StackItem_copyValue(StackItem* item);
StackItem* StackItem_clone(StackItem* item);

// Stack
Stack* Stack_create();
void Stack_dispose(Stack* stack);
void Stack_push(Stack* stack, StackItem* item);
StackItem* Stack_peek(Stack* stack);
StackItem* Stack_pop(Stack* stack);

// Thread
Thread* newThread(Bytecode* main);

// Function
Function* newFunction(char* name, long address);
void Function_dispose(Function* function);

// Object
Object* newObject(char* name);
void Object_dispose(Object* object);
void Object_Child_dispose(Object* object);

#endif//STACK_H
