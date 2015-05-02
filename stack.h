#ifndef STACK_H
#define STACK_H

#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "gc.h"
#include "error.h"


typedef enum {
	Type_CHAR,
	Type_INT,
	Type_FLOAT,
	Type_STRING,
	Type_STREAM,
	Type_FILE_STREAM,
	Type_LOCAL_INDEX,
	Type_LOCAL_FRAME,
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

void StackItem_dispose(StackItem* item);
StackItem* Stack_pop(Stack* stack);

void LocalIndex_dispose(LocalIndex* index) {
	//printf("disposing index: %10lX\n", index);
	StackItem_dispose(index->value);
	free(index);
}
void LocalFrame_dispose(LocalFrame* frame) {
	//printf("disposing frame: %10lX\n", frame);
	free(frame);
}
void StackItem_dispose(StackItem* item) {
	//printf("disposing item: %10lX\n", item);
	switch (item->type) {
		case Type_STREAM : {
			free(item);
			return;}
		case Type_FILE_STREAM : {
			free(item);
			return;}
		case Type_LOCAL_INDEX : {
			LocalIndex_dispose(item->item);
			free(item);
			return;}
		case Type_LOCAL_FRAME : {
			LocalFrame_dispose(item->item);
			free(item);
			return;}
		case Type_NULL : {
			free(item);
			return;}
		default : {
			free(item->item);
			free(item);
			return;}
	}
}
void Stack_dispose(Stack* stack) {
	//printf("disposing stack: %10lX\n", stack);
	while (stack->length > 0) {
		StackItem* item = Stack_pop(stack);
		StackItem_dispose(item);
	}
	free(stack->data);
	free(stack);
}

char* typeName(Type type) {
	switch (type) {
		case Type_INT: return "Int";
		case Type_FLOAT: return "Float";
		case Type_CHAR: return "Char";
		case Type_STRING: return "String";
		case Type_STREAM: return "Stream";
		case Type_FILE_STREAM: return "Stream";
		case Type_LOCAL_INDEX: return "LocalIndex";
		case Type_LOCAL_FRAME: return "LocalFrame";
		case Type_NULL: return "NULL";
		default: return "NA";
	}
}
char* stringValue(StackItem* item);
char* StackItem_toString(StackItem* item);

char* LocalIndex_toString(LocalIndex* item) {
	char* name = item->name;
	char* value = StackItem_toString(item->value);
	int length = strlen(name) + strlen(value) + 2;
	char* out = malloc(length * sizeof(char));
	out[0] = (char) 0;
	strcat(out, name);
	strcat(out, ":");
	strcat(out, value);
	free(value);
	return out;
}

char* LocalFrame_toString(LocalFrame* item) {
	char* out = malloc(sizeof(char));
	sprintf(out, "%d+%d", item->start, item->length);
	return out;
}

StackItem* newStackItem(Type type, void* item) {
	StackItem* out = malloc(sizeof(StackItem));
	if ((out = malloc(sizeof(StackItem))) == NULL) {
		errorMessage = "Stack allocation failed";
		longjmp(errBuf, 1);
	}
	//printf("creating item: %10lX\n", out);
	out->type = type;
	out->item = item;
	return out;
}

void* StackItem_copyValue(StackItem* item) {
	int length = 0;
	if (item->type == Type_INT) length = sizeof(int);
	else if (item->type == Type_FLOAT) length = sizeof(float);
	else if (item->type == Type_CHAR) length = sizeof(char);
	else if (item->type == Type_STRING) length = (strlen(item->item) + 1) * sizeof(char);
	else if (item->type == Type_STREAM || item->type == Type_FILE_STREAM) {
		return item->item;
	}
	void* out = malloc(length);
	memcpy(out, item->item, length);
	return out;
}

StackItem* StackItem_clone(StackItem* item) {
	void* value = StackItem_copyValue(item);
	return newStackItem(item->type, value);
}

char* stringValue(StackItem* item) {
	char* out = calloc(1, sizeof(char));
	int length = -1;
	if (item == NULL) printf("\nitem is null\n");
	switch (item->type) {
		case Type_INT: {
			int value = *(int*) item->item;
			int length = 0;
			if (value == 0) length = 1;
			else length = (int) (log10(value) + 1);
			if (length < 0) length ++;
			out = realloc(out, (length + 1) * sizeof(char));
			length = sprintf(out, "%d", value);
			break; }
		case Type_FLOAT: {
			length = sprintf(out, "%f", *((float*)item->item));
			break; }
		case Type_CHAR: {
			length = sprintf(out, "%c", *((char*)item->item));
			break; }
		case Type_STRING: {
			length = strlen(item->item);
			out = realloc(out, (length + 3) * sizeof(char));
			out[0] = '"';
			int mod = 1;
			for (int i = 0 ; i < length ; i ++) {
				char c = (char) ((char*)item->item)[i];
				if (c == '\n') {
					out[i + mod] = '\\';
					out = realloc(out, (length + (++ mod) + 2) * sizeof(char));
					out[i + mod] = 'n';
				} else if (c == '\t') {
					out[i + mod] = '\\';
					out = realloc(out, (length + (++ mod) + 2) * sizeof(char));
					out[i + mod] = 't';
				} else if (c == '\r') {
					out[i + mod] = '\\';
					out = realloc(out, (length + (++ mod) + 2) * sizeof(char));
					out[i + mod] = 'r';
				} else {
					out[i + mod] = c;
				}
			}
			out[length + mod] = '"';
			out[length + mod + 1] = (char) 0;
			break; }
		case Type_STREAM: {
			int value = *(int*) item->item;
			int length = 0;
			if (value == 0) length = 1;
			else length = (int) (log10(value) + 1);
			if (length < 0) length ++;
			out = realloc(out, (length + 1) * sizeof(char));
			length = sprintf(out, "%d", value);
			break;}
		case Type_FILE_STREAM: {
			void* value = item->item;
			out = realloc(out, 10 * sizeof(char));
			length = sprintf(out, "%8p", value);
			break;}
		case Type_LOCAL_INDEX: {
			char* value = LocalIndex_toString((LocalIndex*)item->item);
			out = realloc(out, (strlen(value) + 1) * sizeof(char));
			length = sprintf(out, "%s", value);
			free(value);
			break; }
		case Type_LOCAL_FRAME: {
			char* value = LocalFrame_toString((LocalFrame*)item->item);
			length = sprintf(out, "%s", value);
			free(value);
			break; }
		default: return "NA";
	}
	//printf("\nlength at %8X = %d\n", &length, length);
	//printf("\nallocation size: %d\n", length * sizeof(char));
	//char* out = malloc((length + 1) * sizeof(char));
	//strcpy(out, temp);
	//free(temp);
	return out;
}

char* StackItem_toString(StackItem* item) {
	char* out = malloc(sizeof(char));
	if (item->type == Type_NULL) {
		sprintf(out, "NULL");
		return out;
	}
	int length = -1;
	char* _typeName = typeName(item->type);
	char* _stringValue = stringValue(item);
	length = strlen(_typeName) + strlen(_stringValue) + 3;
	out = realloc(out, length * sizeof(char));
	out[0] = (char) 0;
	strcat(out, _typeName);
	strcat(out, "(");
	strcat(out, _stringValue);
	strcat(out, ")");
	out[length - 1] = (char) 0;
	free(_stringValue);
	return out;
}

Stack* Stack_create() {
	Stack* out = malloc(sizeof(Stack));
	//printf("creating stack: %10lX\n", out);
	out->length = 0;
	out->data = malloc(sizeof(StackItem));
	out->data->type = Type_NULL;
	out->data->item = NULL;
	return out;
}

void Stack_push(Stack* stack, StackItem* item) {
	stack->length ++;
	item->next = stack->data;
	stack->data = item;
}

StackItem* Stack_peek(Stack* stack) {
    if (stack->length <= 0) {
        fprintf(stderr, "Stack underflow error!!!\n");
		longjmp(errBuf, 1);
    }
	return stack->data;
}

StackItem* Stack_pop(Stack* stack) {
	if (stack->length <= 0) {
		fprintf(stderr, "Stack underflow error!!!\n");
		longjmp(errBuf, 1);
	}
	StackItem* out = stack->data;
	stack->length --;
	stack->data = stack->data->next;
	return out;
}

#endif
