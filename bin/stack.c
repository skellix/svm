#include "stack.h"
#include "bytecode.h"

void LocalIndex_dispose(LocalIndex* index) {
	StackItem_dispose(index->value);
	free(index);
}

void LocalFrame_dispose(LocalFrame* frame) {
	free(frame);
}

void Object_dispose(Object* object) {
	free(object->name);
	for (int i = 0 ; i < object->numVars ; i ++) {
		StackItem index = object->vars[i];
		LocalIndex* var = index.item;
		free(var->name);
		StackItem_dispose(var->value);
		free(var);
	}
	free(object->vars);
	for (int i = 0 ; i < object->numFunctions ; i ++) {
		Function function = object->functions[i];
		free(function.name);
	}
	free(object->functions);
	free(object);
}

void Object_Child_dispose(Object* object) {
	for (int i = 0 ; i < object->numVars ; i ++) {
		StackItem index = object->vars[i];
		LocalIndex* var = index.item;
		StackItem_dispose(var->value);
		free(var);
	}
	free(object->vars);
	free(object);
}

void Array_dispose(Array* array) {
	for (int i = 0 ; i < array->length ; i ++) {
		StackItem* item = &array->data[i];
		switch (item->type) {
			case Type_LOCAL_INDEX : {
				LocalIndex_dispose(item->item);
				break;}
			case Type_LOCAL_FRAME : {
				LocalFrame_dispose(item->item);
				break;}
			case Type_ARRAY : {
				Array_dispose(item->item);
				break;}
			case Type_ARRAY_REF : {
				break;}
			case Type_STACK : {
				Array_dispose(item->item);
				break;}
			case Type_STACK_REF : {
				break;}
			case Type_FILE_STREAM : {
				if (item->item != stdin) {
					free(item->item);
				}
				break;}
			case Type_OBJECT : {
				Object_dispose(item->item);
				break;}
			case Type_OBJECT_REF : {
				break;}
			case Type_OBJECT_CHILD : {
				Object_Child_dispose(item->item);
				break;}
			case Type_OBJECT_CHILD_REF : {
				break;}
			default : {
				free(item->item);
				break;}
		}
	}
	free(array->data);
	free(array);
}

void StackItem_dispose(StackItem* item) {
	switch (item->type) {
		case Type_LOCAL_INDEX : {
			LocalIndex_dispose(item->item);
			break;}
		case Type_LOCAL_FRAME : {
			LocalFrame_dispose(item->item);
			break;}
		case Type_ARRAY : {
			Array_dispose(item->item);
			break;}
		case Type_ARRAY_REF : {
			break;}
		case Type_STACK : {
			Array_dispose(item->item);
			break;}
		case Type_STACK_REF : {
			break;}
		case Type_FILE_STREAM : {
			if (item->item != stdin) {
				free(item->item);
			}
			break;}
		case Type_OBJECT : {
			Object_dispose(item->item);
			break;}
		case Type_OBJECT_REF : {
			break;}
		case Type_OBJECT_CHILD : {
			Object_Child_dispose(item->item);
			break;}
		case Type_OBJECT_CHILD_REF : {
			break;}
		default : {
			free(item->item);
			break;}
	}
	free(item);
}

void Stack_dispose(Stack* stack) {
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
		case Type_ARRAY_REF:
		case Type_ARRAY: return "Array";
		case Type_STACK_REF:
		case Type_STACK: return "Stack";
		case Type_OBJECT_CHILD_REF:
		case Type_OBJECT_CHILD:
		case Type_OBJECT_REF:
		case Type_OBJECT: return "Object";
		case Type_FUNCTION: return "Function";
		case Type_FILE_STREAM: return "Stream";
		case Type_LOCAL_INDEX: return "LocalIndex";
		case Type_LOCAL_FRAME: return "LocalFrame";
		case Type_NULL: return "NULL";
		default: return "NA";
	}
}

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
	sprintf(out, "%X+%X", item->start, item->length);
	return out;
}

StackItem* newStackItem(Type type, void* item) {
	StackItem* out;
	if ((out = malloc(sizeof(StackItem))) == NULL) {
		errorMessage = "Stack allocation failed";
		longjmp(errBuf, 1);
	}
	out->type = type;
	out->item = item;
	return out;
}

Array* newArray() {
	Array* out = malloc(sizeof(Array));
	out->length = 0;
	out->data = malloc(out->length * sizeof(StackItem));
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
	} else if (item->type == Type_ARRAY || item->type == Type_ARRAY_REF) {
		Array* array = item->item;
		Array* out = newArray();
		out->length = array->length;
		out->data = realloc(out->data, out->length * sizeof(StackItem));
		for (int i = 0 ; i < out->length ; i ++) {
			out->data[i].type = array->data[i].type;
			out->data[i].item = StackItem_copyValue(&array->data[i]);
		}
		return out;
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
	int length = 0;
	if (item == NULL) printf("\nitem is null\n");
	switch (item->type) {
		case Type_INT: {
			int value = *(int*) item->item;
			int length = 11;
			out = realloc(out, (length + 1) * sizeof(char));
			memset(out, 0, length + 1);
			length = sprintf(out, "%d", value);
			break; }
		case Type_FLOAT: {
			float value = *(float*) item->item;
			int length = 16;
			out = realloc(out, (length + 1) * sizeof(char));
			memset(out, 0, length + 1);
			length = sprintf(out, "%.6f", value);
			break; }
		case Type_CHAR: {
			out = realloc(out, 2 * sizeof(char));
			length = sprintf(out, "%c", *(char*) item->item);
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
			int length = 11;
			out = realloc(out, (length + 1) * sizeof(char));
			memset(out, 0, length + 1);
			length = sprintf(out, "%d", value);
			break;}
		case Type_ARRAY_REF:
		case Type_ARRAY: {
			Array* array = item->item;
			length = 1;
			out = realloc(out, length * sizeof(char));
			//int index = 0;
			for (int i = 0 ; i < array->length ; i ++) {
				char* itemVal = StackItem_toString(&array->data[i]);
				length += strlen(itemVal) + 2;
				out = realloc(out, length * sizeof(char));
				strcat(out, itemVal);
				if (i + 1 < array->length) {
					strcat(out, ", ");
				}
				free(itemVal);
			}
			break;}
		case Type_OBJECT_CHILD:
		case Type_OBJECT_CHILD_REF:
		case Type_OBJECT_REF:
		case Type_OBJECT: {
			Object* object = item->item;
			length = 1;
			char* varLabel = ":{Variables(";
			length += strlen(object->name) + strlen(varLabel);
			out = realloc(out, length * sizeof(char));
			strcat(out, object->name);
			strcat(out, varLabel);
			for (int i = 0 ; i < object->numVars ; i ++) {
				char* itemVal = StackItem_toString(&object->vars[i]);
				length += strlen(itemVal) + 2;
				out = realloc(out, length * sizeof(char));
				strcat(out, itemVal);
				if (i + 1 < object->numVars) {
					strcat(out, ", ");
				}
				free(itemVal);
			}
			char* funcLabel = "), Functions(";
			length += strlen(funcLabel);
			out = realloc(out, length * sizeof(char));
			strcat(out, funcLabel);
			for (int i = 0 ; i < object->numFunctions ; i ++) {
				Function func = object->functions[i];
				length += strlen(func.name) + 4;
				out = realloc(out, length * sizeof(char));
				strcat(out, "\"");
				strcat(out, func.name);
				strcat(out, "\"");
				if (i + 1 < object->numFunctions) {
					strcat(out, ", ");
				}
			}
			char* endLabel = ")}";
			length += strlen(endLabel);
			out = realloc(out, length * sizeof(char));
			strcat(out, endLabel);
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
	return out;
}

char* StackItem_toString(StackItem* item) {
	char* out = malloc(sizeof(char));
	if (item->type == Type_NULL) {
		out = realloc(out, 5 * sizeof(char));
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

Thread* newThread(Bytecode* main) {
	Thread* out = malloc(sizeof(Thread));
	out->stack = Stack_create();
	out->locals = Stack_create();
	out->localFrame = Stack_create();
	out->classes = Stack_create();
	out->functions = Stack_create();
	out->running = calloc(1, sizeof(int));
	out->currentLocation = calloc(1, sizeof(long));
	out->main = main;
	return out;
}


Object* newObject(char* name) {
	Object* out = calloc(1, sizeof(Object));
	out->name = calloc(strlen(name) + 1, sizeof(char));
	strcpy(out->name, name);
	out->vars = malloc(0);
	out->functions = malloc(0);
	return out;
}
