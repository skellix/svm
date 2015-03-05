#ifndef STACK_H
#define STACK_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <gc.h>


typedef enum {
	Type_CHAR,
	Type_INT,
	Type_FLOAT,
	Type_STRING,
} Type;

char* typeName(Type type) {
	switch (type) {
		case Type_INT: return "Int";
		case Type_FLOAT: return "Float";
		case Type_CHAR: return "Char";
		case Type_STRING: return "String";
		default: return "NA";
	}
}

typedef struct {
	Type type;
	void* item;
} StackItem;

StackItem* newStackItem(Type type, void* item) {
	StackItem* out = GC_MALLOC(sizeof(StackItem));
	out->type = type;
	out->item = item;
	return out;
}

char* stringValue(StackItem* item) {
	char* temp = GC_MALLOC(sizeof(char));
	int length = -1;
	switch (item->type) {
		case Type_INT: {length = snprintf(temp, 10000, "%d", *((int*)item->item)); break;}
		case Type_FLOAT: {length = snprintf(temp, 10000, "%f", *((float*)item->item)); break;}
		case Type_CHAR: {length = snprintf(temp, 10000, "%c", *((char*)item->item)); break;}
		case Type_STRING: {length = strlen(item->item); temp = GC_REALLOC(temp, sizeof(char) * length); strcpy(temp, (char*)item->item); break;}
		default: return "NA";
	}
	char* out = GC_MALLOC(sizeof(char)*length);
	strcpy(out, temp);
	GC_free(temp);
	return out;
}

char* StackItem_toString(StackItem* item) {
	char* temp = GC_MALLOC(sizeof(char));
	int length = -1;
	length = snprintf(temp, 10000, "%s(%s)", typeName(item->type), stringValue(item));
	char* out = GC_MALLOC(sizeof(char)*length);
	strcpy(out, temp);
	GC_free(temp);
	return out;
}

typedef struct {
	int length;
	StackItem** data;
} Stack;

Stack* Stack_create() {
	Stack* out = GC_MALLOC(sizeof(Stack));
	out->length = 0;
	out->data = GC_MALLOC(sizeof(void*));
	return out;
}

void Stack_push(Stack* stack, StackItem* item) {
	stack->length ++;
	if ((stack->data = GC_REALLOC(stack->data, sizeof(StackItem*)*stack->length)) == NULL) {
		fprintf(stderr, "stack reallocation failed!!!\n");
		exit(1);
	}
	stack->data[stack->length - 1] = item;
}

StackItem* Stack_peek(Stack* stack) {
	return stack->data[stack->length - 1];
}

StackItem* Stack_pop(Stack* stack) {
	StackItem* out = stack->data[stack->length - 1];
	stack->length --;
	if (stack->length < 0) {
		fprintf(stderr, "Stack underflow error!!!\n");
		exit(1);
	}
	if ((stack->data = GC_REALLOC(stack->data, sizeof(StackItem*)*stack->length)) == NULL) {
		fprintf(stderr, "stack reallocation failed!!!\n");
		exit(1);
	}
	return out;
}

#endif
