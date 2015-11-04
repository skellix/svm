#include "interrupt.h"

#include <dlfcn.h>
#include "stdfuncs.h"
#include "opcodes.h"


StackItem* int_add(StackItem* lOperand, StackItem* rOperand) {
	Type oType = rOperand->type > lOperand->type ? rOperand->type : lOperand->type;
	switch (oType) {
		case Type_STRING: {
			if (lOperand->type == rOperand->type) {
				char* val = calloc(strlen((char*) lOperand->item) + strlen((char*) rOperand->item) + 1, sizeof(char));
				sprintf(val, "%s%s", (char*) lOperand->item, (char*) rOperand->item);
				StackItem_dispose(lOperand);
				StackItem_dispose(rOperand);
				return newStackItem(Type_STRING, val);
			}
			break;}
		case Type_ARRAY_REF:
		case Type_ARRAY: {
			if (lOperand->type == Type_ARRAY || lOperand->type == Type_ARRAY_REF) {
				Array* array = lOperand->item;
				if (rOperand->type == Type_INT) {
					int amt = *(int*) rOperand->item;
					if (amt >= 0) {
						int newLength = array->length + amt;
						array->data = realloc(array->data, newLength * sizeof(StackItem));
						for (; array->length < newLength ; array->length ++) {
							array->data[array->length].type = Type_NULL;
							array->data[array->length].item = NULL;
						}
						lOperand->item = array;
						return lOperand;
					}
				}
			}
			break;}
		case Type_CHAR: {
			char* val = malloc(sizeof(char));
			*val = *(char*) lOperand->item + *(char*) rOperand->item;
			StackItem_dispose(lOperand);
			StackItem_dispose(rOperand);
			return newStackItem(oType, val);}
		case Type_INT: {
			int* val = malloc(sizeof(int));
			*val = *(int*) lOperand->item + *(int*) rOperand->item;
			StackItem_dispose(lOperand);
			StackItem_dispose(rOperand);
			return newStackItem(oType, val);}
		case Type_FLOAT: {
			float* val = malloc(sizeof(float));
			float lo = 0;
			if (lOperand->type == Type_INT || lOperand->type == Type_CHAR) {
				int lot = *(int*) lOperand->item;
				lo = (float) lot;
			} else {
				lo = *(float*) lOperand->item;
			}
			float ro = 0;
			if (rOperand->type == Type_INT || rOperand->type == Type_CHAR) {
				int rot = *(int*) rOperand->item;
				ro = (float) rot;
			} else {
				ro = *(float*) rOperand->item;
			}
			*val = lo + ro;
			StackItem_dispose(lOperand);
			StackItem_dispose(rOperand);
			return newStackItem(oType, val);}
	}
	return newStackItem(Type_NULL, NULL);
}
StackItem* int_subtract(StackItem* lOperand, StackItem* rOperand) {
	Type oType = rOperand->type > lOperand->type ? rOperand->type : lOperand->type;
	switch (oType) {
		case Type_STRING: {
			fprintf(stderr, "strings types can't be used in math!!!\n");
			exit(1);
			break;}
		case Type_ARRAY_REF:
		case Type_ARRAY: {
			if (lOperand->type == Type_ARRAY || lOperand->type == Type_ARRAY_REF) { 
				Array* array = lOperand->item;
				if (rOperand->type == Type_INT) {
					int amt = *(int*) rOperand->item;
					if (amt >= 0) {
						array->length -= amt;
						array->data = realloc(array->data, array->length * sizeof(StackItem));
						lOperand->item = array;
						return lOperand;
					}
				}
			}
			break;}
		case Type_CHAR: {
			char* val = malloc(sizeof(char));
			*val = *(char*) lOperand->item - *(char*) rOperand->item;
			StackItem_dispose(lOperand);
			StackItem_dispose(rOperand);
			return newStackItem(oType, val);}
		case Type_INT: {
			int* val = malloc(sizeof(int));
			*val = *(int*) lOperand->item - *(int*) rOperand->item;
			StackItem_dispose(lOperand);
			StackItem_dispose(rOperand);
			return newStackItem(oType, val);}
		case Type_FLOAT: {
			float* val = malloc(sizeof(float));
			float lo = 0;
			if (lOperand->type == Type_INT || lOperand->type == Type_CHAR) {
				int lot = *(int*) lOperand->item;
				lo = (float) lot;
			} else {
				lo = *(float*) lOperand->item;
			}
			float ro = 0;
			if (rOperand->type == Type_INT || rOperand->type == Type_CHAR) {
				int rot = *(int*) rOperand->item;
				ro = (float) rot;
			} else {
				ro = *(float*) rOperand->item;
			}
			*val = lo - ro;
			StackItem_dispose(lOperand);
			StackItem_dispose(rOperand);
			return newStackItem(oType, val);}
	}
	return newStackItem(Type_NULL, NULL);
}
StackItem* int_multiply(StackItem* lOperand, StackItem* rOperand) {
	Type oType = rOperand->type > lOperand->type ? rOperand->type : lOperand->type;
	if (oType == Type_STRING) {
		fprintf(stderr, "strings types can't be used in math!!!\n");
		exit(1);
	}
	if (oType == Type_CHAR) {
		char* val = malloc(sizeof(char));
		*val = *(char*) lOperand->item * *(char*) rOperand->item;
		StackItem_dispose(lOperand);
		StackItem_dispose(rOperand);
		return newStackItem(oType, val);
	} else if (oType == Type_INT) {
		int* val = malloc(sizeof(int));
		*val = *(int*) lOperand->item * *(int*) rOperand->item;
		StackItem_dispose(lOperand);
		StackItem_dispose(rOperand);
		return newStackItem(oType, val);
	} else if (oType == Type_FLOAT) {
		float* val = malloc(sizeof(float));
		float lo = 0;
		if (lOperand->type == Type_INT || lOperand->type == Type_CHAR) {
			int lot = *(int*) lOperand->item;
			lo = (float) lot;
		} else {
			lo = *(float*) lOperand->item;
		}
		float ro = 0;
		if (rOperand->type == Type_INT || rOperand->type == Type_CHAR) {
			int rot = *(int*) rOperand->item;
			ro = (float) rot;
		} else {
			ro = *(float*) rOperand->item;
		}
		*val = lo * ro;
		StackItem_dispose(lOperand);
		StackItem_dispose(rOperand);
		return newStackItem(oType, val);
	}
	return newStackItem(Type_NULL, NULL);
}
StackItem* int_divide(StackItem* lOperand, StackItem* rOperand) {
	Type oType = rOperand->type > lOperand->type ? rOperand->type : lOperand->type;
	if (oType == Type_STRING) {
		fprintf(stderr, "strings types can't be used in math!!!\n");
		exit(1);
	}
	if (oType == Type_CHAR) {
		char* val = malloc(sizeof(char));
		*val = *(char*) lOperand->item / *(char*) rOperand->item;
		StackItem_dispose(lOperand);
		StackItem_dispose(rOperand);
		return newStackItem(oType, val);
	} else if (oType == Type_INT) {
		int* val = malloc(sizeof(int));
		*val = *(int*) lOperand->item / *(int*) rOperand->item;
		StackItem_dispose(lOperand);
		StackItem_dispose(rOperand);
		return newStackItem(oType, val);
	} else if (oType == Type_FLOAT) {
		float* val = malloc(sizeof(float));
		float lo = 0;
		if (lOperand->type == Type_INT || lOperand->type == Type_CHAR) {
			int lot = *(int*) lOperand->item;
			lo = (float) lot;
		} else {
			lo = *(float*) lOperand->item;
		}
		float ro = 0;
		if (rOperand->type == Type_INT || rOperand->type == Type_CHAR) {
			int rot = *(int*) rOperand->item;
			ro = (float) rot;
		} else {
			ro = *(float*) rOperand->item;
		}
		*val = lo / ro;
		StackItem_dispose(lOperand);
		StackItem_dispose(rOperand);
		return newStackItem(oType, val);
	}
	return newStackItem(Type_NULL, NULL);
}
StackItem* int_mod(StackItem* lOperand, StackItem* rOperand) {
	Type oType = rOperand->type > lOperand->type ? rOperand->type : lOperand->type;
	if (oType == Type_STRING) {
		fprintf(stderr, "strings types can't be used in math!!!\n");
		exit(1);
	}
	int* val = malloc(sizeof(int));
	if (oType == Type_CHAR) {
		*val = *(char*) lOperand->item % *(char*) rOperand->item;
	} else if (oType == Type_INT) {
		*val = *(int*) lOperand->item % *(int*) rOperand->item;
	} else if (oType == Type_FLOAT) {
		float lo = 0;
		if (lOperand->type == Type_INT || lOperand->type == Type_CHAR) {
			int lot = *(int*) lOperand->item;
			lo = (float) lot;
		} else {
			lo = *(float*) lOperand->item;
		}
		float ro = 0;
		if (rOperand->type == Type_INT || rOperand->type == Type_CHAR) {
			int rot = *(int*) rOperand->item;
			ro = (float) rot;
		} else {
			ro = *(float*) rOperand->item;
		}
		*val = (int) lo % (int) ro;
	}
	StackItem_dispose(lOperand);
	StackItem_dispose(rOperand);
	return newStackItem(Type_INT, val);
}
StackItem* int_less(StackItem* lOperand, StackItem* rOperand) {
	Type oType = rOperand->type > lOperand->type ? rOperand->type : lOperand->type;
	if (oType == Type_STRING) {
		fprintf(stderr, "strings types can't be used in math!!!\n");
		exit(1);
	}
	int* val = malloc(sizeof(int));
	if (oType == Type_CHAR) {
		*val = *(char*) lOperand->item < *(char*) rOperand->item ? 1 : 0;	
		StackItem_dispose(lOperand);
		StackItem_dispose(rOperand);
		return newStackItem(oType, val);
	} else if (oType == Type_INT) {
		*val = *(int*) lOperand->item < *(int*) rOperand->item ? 1 : 0;
		StackItem_dispose(lOperand);
		StackItem_dispose(rOperand);
		return newStackItem(oType, val);
	} else if (oType == Type_FLOAT) {
		float lo = 0;
		if (lOperand->type == Type_INT || lOperand->type == Type_CHAR) {
			int lot = *(int*) lOperand->item;
			lo = (float) lot;
		} else {
			lo = *(float*) lOperand->item;
		}
		float ro = 0;
		if (rOperand->type == Type_INT || rOperand->type == Type_CHAR) {
			int rot = *(int*) rOperand->item;
			ro = (float) rot;
		} else {
			ro = *(float*) rOperand->item;
		}
		*val = lo < ro ? 1 : 0;
		StackItem_dispose(lOperand);
		StackItem_dispose(rOperand);
		return newStackItem(Type_INT, val);
	}
	return newStackItem(Type_NULL, NULL);
}
StackItem* int_greater(StackItem* lOperand, StackItem* rOperand) {
	Type oType = rOperand->type > lOperand->type ? rOperand->type : lOperand->type;
	int* val = malloc(sizeof(int));
	if (oType == Type_STRING) {
		fprintf(stderr, "strings types can't be used in math!!!\n");
		exit(1);
	} else if (oType == Type_CHAR) {
		*val = *(char*) lOperand->item > *(char*) rOperand->item ? 1 : 0;
		StackItem_dispose(lOperand);
		StackItem_dispose(rOperand);
		return newStackItem(oType, val);
	} else if (oType == Type_INT) {
		*val = *(int*) lOperand->item > *(int*) rOperand->item ? 1 : 0;
		StackItem_dispose(lOperand);
		StackItem_dispose(rOperand);
		return newStackItem(oType, val);
	} else if (oType == Type_FLOAT) {
		float lo = 0;
		if (lOperand->type == Type_INT || lOperand->type == Type_CHAR) {
			int lot = *(int*) lOperand->item;
			lo = (float) lot;
		} else {
			lo = *((float*)lOperand->item);
		}
		float ro = 0;
		if (rOperand->type == Type_INT || rOperand->type == Type_CHAR) {
			int rot = *(int*) rOperand->item;
			ro = (float) rot;
		} else {
			ro = *(float*) rOperand->item;
		}
		*val = lo > ro ? 1 : 0;
		StackItem_dispose(lOperand);
		StackItem_dispose(rOperand);
		return newStackItem(Type_INT, val);
	}
	return newStackItem(Type_NULL, NULL);
}
StackItem* int_logic_not(StackItem* item) {
	switch (item->type) {
		case Type_STRING: {
			fprintf(stderr, "boolean operations cannot be performed on a string!!!\n");
			exit(0); }
		case Type_CHAR: {
			char* val = malloc(sizeof(char));
			*val = *(char*) item->item == 0 ? (char) 1 : (char) 0;
			StackItem_dispose(item);
			return newStackItem(Type_CHAR, val); }
		case Type_INT: {
			int* val = malloc(sizeof(int));
			*val = *(int*) item->item == 0 ? 1 : 0;
			StackItem_dispose(item);
			return newStackItem(Type_INT, val); }
		case Type_FLOAT: {
			float* val = malloc(sizeof(float));
			*val = *(float*) item->item == 0 ? (float) 1 : (float) 0;
			StackItem_dispose(item);
			return newStackItem(Type_FLOAT, val); }
        default: {return newStackItem(Type_NULL, NULL);}
	}
}
StackItem* int_equal(StackItem* lOperand, StackItem* rOperand) {
	Type oType = rOperand->type > lOperand->type ? rOperand->type : lOperand->type;
	int* val = malloc(sizeof(int));
	switch (oType) {
		case Type_STRING: {
			if (rOperand->type == lOperand->type) {
				*val = strcmp(lOperand->item, rOperand->item) == 0 ? 1 : 0;
				StackItem_dispose(lOperand);
				StackItem_dispose(rOperand);
				return newStackItem(Type_INT, val);
			}}
		case Type_CHAR: {
			*val = *(char*) lOperand->item == *(char*) rOperand->item ? 1 : 0;
			StackItem_dispose(lOperand);
			StackItem_dispose(rOperand);
			return newStackItem(Type_INT, val); }
		case Type_INT: {
			*val = *(int*) lOperand->item == *(int*) rOperand->item ? 1 : 0;
			StackItem_dispose(lOperand);
			StackItem_dispose(rOperand);
			return newStackItem(Type_INT, val); }
		case Type_FLOAT: {
			float lo = 0;
			if (lOperand->type == Type_INT || lOperand->type == Type_CHAR) {
				int lot = *(int*) lOperand->item;
				lo = (float) lot;
			} else lo = *(float*) lOperand->item;
			float ro = 0;
			if (rOperand->type == Type_INT || rOperand->type == Type_CHAR) {
				int rot = *(int*) rOperand->item;
				ro = (float) rot;
			} else ro = *(float*) rOperand->item;
			StackItem_dispose(lOperand);
			StackItem_dispose(rOperand);
			*val = lo == ro ? 1 : 0;
			return newStackItem(Type_INT, val); }
        default: {return newStackItem(Type_NULL, NULL);}
	}
}
StackItem* int_and(StackItem* lOperand, StackItem* rOperand) {
	Type oType = rOperand->type > lOperand->type ? rOperand->type : lOperand->type;
	switch (oType) {
		case Type_STRING: {
			fprintf(stderr, "strings types can't be used in math!!!\n");
			exit(1); }
		case Type_CHAR: {
			char* val = malloc(sizeof(char));
			*val = *(char*) lOperand->item & *(char*) rOperand->item;
			StackItem_dispose(lOperand);
			StackItem_dispose(rOperand);
			return newStackItem(oType, val); }
		case Type_INT: {
			int* val = malloc(sizeof(int));
			*val = *(int*) lOperand->item & *(int*) rOperand->item;
			StackItem_dispose(lOperand);
			StackItem_dispose(rOperand);
			return newStackItem(oType, val); }
		case Type_FLOAT: {
			float* val = malloc(sizeof(float));
			float lo = 0;
			if (lOperand->type == Type_INT || lOperand->type == Type_CHAR) {
				int lot = *(int*) lOperand->item;
				lo = (float) lot;
			} else lo = *(float*) lOperand->item;
			float ro = 0;
			if (rOperand->type == Type_INT || rOperand->type == Type_CHAR) {
				int rot = *(int*) rOperand->item;
				ro = (float) rot;
			} else ro = *(float*) rOperand->item;
			StackItem_dispose(lOperand);
			StackItem_dispose(rOperand);
			*val = (float) ((int) lo & (int) ro);
			return newStackItem(oType, val); }
        default: {return newStackItem(Type_NULL, NULL);}
	}
}
StackItem* int_or(StackItem* lOperand, StackItem* rOperand) {
	Type oType = rOperand->type > lOperand->type ? rOperand->type : lOperand->type;
	switch (oType) {
		case Type_STRING: {
			fprintf(stderr, "strings types can't be used in math!!!\n");
			exit(1); }
		case Type_CHAR: {
			char* val = malloc(sizeof(char));
			*val = *(char*) lOperand->item | *(char*) rOperand->item;
			StackItem_dispose(lOperand);
			StackItem_dispose(rOperand);
			return newStackItem(oType, val); }
		case Type_INT: {
			int* val = malloc(sizeof(int));
			*val = *(int*) lOperand->item | *(int*) rOperand->item;
			StackItem_dispose(lOperand);
			StackItem_dispose(rOperand);
			return newStackItem(oType, val); }
		case Type_FLOAT: {
			float* val = malloc(sizeof(float));
			float lo = 0;
			if (lOperand->type == Type_INT || lOperand->type == Type_CHAR) {
				int lot = *(int*) lOperand->item;
				lo = (float) lot;
			} else lo = *(float*) lOperand->item;
			float ro = 0;
			if (rOperand->type == Type_INT || rOperand->type == Type_CHAR) {
				int rot = *(int*) rOperand->item;
				ro = (float) rot;
			} else ro = *(float*) rOperand->item;
			StackItem_dispose(lOperand);
			StackItem_dispose(rOperand);
			*val = (float) ((int) lo | (int) ro);
			return newStackItem(oType, val); }
        default: {return newStackItem(Type_NULL, NULL);}
	}
}
StackItem* int_xor(StackItem* lOperand, StackItem* rOperand) {
	Type oType = rOperand->type > lOperand->type ? rOperand->type : lOperand->type;
	switch (oType) {
		case Type_STRING: {
			fprintf(stderr, "strings types can't be used in math!!!\n");
			exit(1); }
		case Type_CHAR: {
			char* val = malloc(sizeof(char));
			*val = *(char*) lOperand->item ^ *(char*) rOperand->item;
			StackItem_dispose(lOperand);
			StackItem_dispose(rOperand);
			return newStackItem(oType, val); }
		case Type_INT: {
			int* val = malloc(sizeof(int));
			*val = *(int*) lOperand->item ^ *(int*) rOperand->item;
			StackItem_dispose(lOperand);
			StackItem_dispose(rOperand);
			return newStackItem(oType, val); }
		case Type_FLOAT: {
			float* val = malloc(sizeof(float));
			float lo = 0;
			if (lOperand->type == Type_INT || lOperand->type == Type_CHAR) {
				int lot = *(int*) lOperand->item;
				lo = (float) lot;
			} else lo = *(float*) lOperand->item;
			float ro = 0;
			if (rOperand->type == Type_INT || rOperand->type == Type_CHAR) {
				int rot = *(int*) rOperand->item;
				ro = (float) rot;
			} else ro = *(float*) rOperand->item;
			StackItem_dispose(lOperand);
			StackItem_dispose(rOperand);
			*val = (float) ((int) lo ^ (int) ro);
			return newStackItem(oType, val); }
        default: {return newStackItem(Type_NULL, NULL);}
	}
}
StackItem* int_bit_not(StackItem* item) {
	switch (item->type) {
		case Type_CHAR: {
			char* val = malloc(sizeof(char));
			*val = (char) ~(*((char*)item->item));
			StackItem_dispose(item);
			return newStackItem(Type_CHAR, val); }
		case Type_INT: {
			int* val = malloc(sizeof(int));
			*val = ~(*((int*)item->item));
			StackItem_dispose(item);
			return newStackItem(Type_INT, val); }
		case Type_FLOAT: {
			float* val = malloc(sizeof(float));
			float temp = *((float*)item->item);
			*val = (float) ~(int) temp;
			StackItem_dispose(item);
			return newStackItem(Type_FLOAT, val); }
        default: {return newStackItem(Type_NULL, NULL);}
	}
}

void int_array(Thread* thread, long label) {
	StackItem* item = newStackItem(Type_ARRAY, newArray());
	if (thread->stack->length > label) {
		Array* array = item->item;
		//printf("reallocating array to size: %d\n", thread->stack->length - label);
		array->length = thread->stack->length - label;
		array->data = realloc(array->data, array->length * sizeof(StackItem));
		while (thread->stack->length > label) {
			StackItem* item2 = Stack_pop(thread->stack);
			array->data[thread->stack->length].type = item2->type;
			array->data[thread->stack->length].item = StackItem_copyValue(item2);
			StackItem_dispose(item2);
		}
	}
	Stack_push(thread->stack, item);
}

void int_break(Thread* thread, long label) {
	StackItem* frameItem = Stack_peek(thread->localFrame);
	LocalFrame* frame = (LocalFrame*) frameItem->item;
	*thread->currentLocation = frame->start + frame->length;
}

void int_call(Thread* thread, long label) {
	StackItem* funcName = Stack_pop(thread->stack);
	if (thread->stack->length > 0) {
		StackItem* last = Stack_peek(thread->stack);
		if (last->type == Type_OBJECT_CHILD
				|| last->type == Type_OBJECT_CHILD_REF) {
			last = Stack_pop(thread->stack);
			Object* object = last->item;
			for (int i = 0 ; i < object->numFunctions ; i ++) {
				Function func = object->functions[i];
				if (strcmp(func.name, funcName->item) == 0) {
						long location = *thread->currentLocation;
					*thread->currentLocation = func.address;
					Stack* oldLocals = thread->locals;
					thread->locals = Stack_create();
					for (int j = 0 ; j < object->numVars ; j ++) {
						StackItem item = object->vars[j];
						Stack_push(thread->locals, &item);
					}
					int frameLabel = thread->localFrame->length;
					exec(thread);
					StackItem* frameItem = Stack_peek(thread->localFrame);
					LocalFrame* frame = frameItem->item;
					long end = frame->start + frame->length;
					while (*thread->currentLocation < end) {
						(*thread->currentLocation) ++;
						exec(thread);
					}
					while (thread->locals->length > 0) {
						Stack_pop(thread->locals);
					}
					Stack_dispose(thread->locals);
					thread->locals = oldLocals;
					*thread->currentLocation = location;
					StackItem_dispose(last);
					return;
				}
			}
			Stack_push(thread->stack, last);
		}
	}
	StackItem* item = Stack_peek(thread->functions);
	for (int i = 0 ; i < thread->functions->length ; i ++) {
		Function* func = item->item;
		if (strcmp(func->name, funcName->item) == 0) {
			long location = *thread->currentLocation;
			*thread->currentLocation = func->address;
			Stack* oldLocals = thread->locals;
			thread->locals = Stack_create();
			int frameLabel = thread->localFrame->length;
			do {
				exec(thread);
				(*thread->currentLocation) ++;
			} while (thread->localFrame->length > frameLabel);
			Stack_dispose(thread->locals);
			thread->locals = oldLocals;
			*thread->currentLocation = location;
			break;
		}
	}
}

void int_class(Thread* thread, long label) {
	StackItem* name = Stack_pop(thread->stack);
	Object* out = newObject(name->item);
	long location = *thread->currentLocation+1;
	int nextOpCall = thread->main->data[location];
	if (nextOpCall == FRAME) {
		int length = *(int*) &thread->main->data[location+1];
		(*thread->currentLocation) ++;
		long end = location + length;
		Stack* oldLocals = thread->locals;
		thread->locals = Stack_create();
		Stack* oldFunctions = thread->functions;
		thread->functions = Stack_create();
		int frameLabel = thread->localFrame->length;
		do {
			exec(thread);
			(*thread->currentLocation) ++;
		} while (*thread->currentLocation < end-1);
		StackItem* frameItem = Stack_pop(thread->localFrame);
		StackItem_dispose(frameItem);
		out->numVars = thread->locals->length;
		out->vars = realloc(out->vars, out->numVars * sizeof(StackItem));
		for (int i = 0 ; i < out->numVars ; i ++) {
			StackItem* item = Stack_pop(thread->locals);
			out->vars[i].type = item->type;
			out->vars[i].item = item->item;
			free(item);
		}
		Stack_dispose(thread->locals);
		thread->locals = oldLocals;
		out->numFunctions = thread->functions->length;
		out->functions = realloc(out->functions, out->numFunctions * sizeof(Function));
		for (int i = 0 ; i < out->numFunctions ; i ++) {
			StackItem* item = Stack_pop(thread->functions);
			Function* func = item->item;
			out->functions[i].name = func->name;
			out->functions[i].address = func->address;
			free(func);
			free(item);
		}
		Stack_dispose(thread->functions);
		thread->functions = oldFunctions;
		*thread->currentLocation = location + length;
	} else {
		errorMessage = "no definition found for object";
		longjmp(errBuf, 1);
	}
	StackItem_dispose(name);
	Stack_push(thread->classes, newStackItem(Type_OBJECT, out));
}

void int_close(Thread* thread, long label) {
	StackItem* stream = Stack_pop(thread->stack);
	if (stream->type == Type_STREAM) {
		void* lib = ((void**)(&(*(int*) stream->item) + 1))[2];
		void (*_closeStream)(StackItem*) = ((void**)(&(*(int*) stream->item) + 1))[1];
		_closeStream(stream);
		dlclose(lib);
	} else if (stream->type == Type_FILE_STREAM) {
		void* lib = ((void**)(&(*(FILE*) stream->item) + 1))[2];
		void (*_closeStream)(StackItem*) = ((void**)(&(*(FILE*) stream->item) + 1))[1];
		_closeStream(stream);
		dlclose(lib);
	}
	StackItem_dispose(stream);
}

void int_continue(Thread* thread, long label) {
	StackItem* frameItem = Stack_peek(thread->localFrame);
	LocalFrame* frame = (LocalFrame*) frameItem->item;
	while (thread->stack->length > frame->stackSize) {
		StackItem* item = Stack_pop(thread->stack);
		StackItem_dispose(item);
	}
	while (thread->locals->length > frame->localCount) {
		StackItem* item = Stack_pop(thread->locals);
		StackItem_dispose(item);
	}
	*thread->currentLocation = frame->start + 4;
}

void int_dumpFrame(Thread* thread, long label) {
	dumpFrame(thread->locals, thread->localFrame, stdout);
}

void int_dumpLocals(Thread* thread, long label) {
	dumpLocals(thread->locals, thread->localFrame, stdout);
}

void int_dumpStack(Thread* thread, long label) {
	dumpStack(thread->stack, stdout);
}

void int_function(Thread* thread, long label) {
	StackItem* name = Stack_pop(thread->stack);
	long location = *thread->currentLocation + 1;
	//short nextCall = thread->main->data[location];
	int nextOpCall = thread->main->data[location];
	if (nextOpCall == FRAME) {
		int length = *(int*) &thread->main->data[location + 1];
		*thread->currentLocation += length + 1;
		Function* funcDef = malloc(sizeof(Function));
		funcDef->name = calloc(strlen((char*) name->item) + 1, sizeof(char));
		strcpy(funcDef->name, name->item);
		funcDef->address = location;
		Stack_push(thread->functions, newStackItem(Type_FUNCTION, funcDef));
	} else {
		errorMessage = calloc(1, sizeof(char));
		sprintf(errorMessage, "no definition found for function '%s'", (char*) name->item);
		longjmp(errBuf, 1);
	}
	StackItem_dispose(name);
}

void int_listClasses(Thread* thread, long label) {
	Array* out = newArray();
	if (thread->classes->length > 0) {
		out->length = thread->classes->length;
		out->data = realloc(out->data, out->length * sizeof(StackItem));
		StackItem* item = Stack_peek(thread->classes);
		for (int i = 0 ; i < thread->classes->length ; i ++) {
			out->data[i].type = Type_STRING;
			char* strVal = StackItem_toString(item);
			out->data[i].item = calloc(strlen(strVal) + 1, sizeof(char));
			strcpy(out->data[i].item, strVal);
			item = item->next;
		}
	}
	Stack_push(thread->stack, newStackItem(Type_ARRAY, out));
}

void int_listFunctions(Thread* thread, long label) {
	Array* out = newArray();
	StackItem* last = Stack_peek(thread->stack);
	if (last->type == Type_OBJECT || last->type == Type_OBJECT_REF
			|| last->type == Type_OBJECT_CHILD
			|| last->type == Type_OBJECT_CHILD_REF) {
		last = Stack_pop(thread->stack);
		Object* object = last->item;
		if (object->numFunctions > 0) {
			out->length = object->numFunctions;
			out->data = realloc(out->data, out->length * sizeof(StackItem));
			for (int i = 0 ; i < out->length ; i ++) {
				out->data[i].type = Type_STRING;
				char* funcName = object->functions[i].name;
				out->data[i].item = calloc(strlen(funcName) + 1, sizeof(char));
				strcpy(out->data[i].item, funcName);
			}
		} 
		StackItem_dispose(last);
	} else {
		if (thread->functions->length > 0) {
			out->length = thread->functions->length;
			out->data = realloc(out->data, out->length * sizeof(StackItem));
			StackItem* item = thread->functions->data;
			for (int i = 0 ; i < out->length ; i ++) {
				out->data[i].type = Type_STRING;
				char* funcName = ((Function*) item->item)->name;
				out->data[i].item = calloc(strlen(funcName) + 1, sizeof(char));
				strcpy(out->data[i].item, funcName);
				item = item->next;
			}
		}
	}
	Stack_push(thread->stack, newStackItem(Type_ARRAY, out));
}

void int_new(Thread* thread, long label) {
	StackItem* name = Stack_pop(thread->stack);
	if (name->type == Type_STRING) {
		StackItem* item = Stack_peek(thread->classes);
		for (int i = 0 ; i < thread->classes->length ; i ++) {
			Object* object = item->item;
			if (strcmp(object->name, name->item) == 0) {
				Object* out = calloc(1, sizeof(Object));
				out->name = object->name;
				out->numVars = object->numVars;
				out->vars = malloc(out->numVars * sizeof(StackItem));
				for (int i = 0 ; i < out->numVars ; i ++) {
					LocalIndex* objectVar = object->vars[i].item;
					out->vars[i].type = Type_LOCAL_INDEX;
					LocalIndex* var = malloc(sizeof(LocalIndex));
					var->name = objectVar->name;
					var->value = StackItem_clone(objectVar->value);
					out->vars[i].item = var;
				}
				out->numFunctions = object->numFunctions;
				out->functions = object->functions;
				Stack_push(thread->stack, newStackItem(Type_OBJECT_CHILD, out));
				return;
			}
			item = item->next;
		}
		errorMessage = realloc(errorMessage, sizeof(char));
		sprintf(errorMessage, "class not found '%s'", (char*) name->item);
		longjmp(errBuf, 1);
	} else {
		errorMessage = "invalid arguments for function new";
		longjmp(errBuf, 1);
	}
}

void int_open(Thread* thread, long label) {
	StackItem* mode = Stack_pop(thread->stack);
	StackItem* address = Stack_pop(thread->stack);
	void* lib = dlopen("libsvm.io.so", RTLD_LAZY);
	StackItem* (*open)(char* address, char* mode) = dlsym(lib, "openStream");
	if (dlerror() != NULL) {
		errorMessage = "unable to load libsvm.debug.so funcs";
		longjmp(errBuf, 1);
	}
	Stack_push(thread->stack, open(address->item, mode->item));
	dlclose(lib);
	StackItem_dispose(address);
	StackItem_dispose(mode);
}

void int_printf(Thread* thread, long label) {
	Stack* argStack = Stack_create();
	while (thread->stack->length > label) {
		Stack_push(argStack, Stack_pop(thread->stack));
	}
	StackItem* format = Stack_pop(argStack);
	if (format->type == Type_STRING) {
		int start = 0, end = -1;
		char* fData = ((char*)format->item);
		int fLen = strlen(fData);
		int arg = 0;
		for (int i = 0 ; i <= fLen ; i ++) {
			if (fData[i] == '%' || i == fLen) {
				end = i;
				int len = end - start;
				char* temp = calloc(len + 1, sizeof(char));
				for (int i = 0 ; i < len ; i ++) {
					temp[i] = fData[start + i];
				}
				temp[len] = 0;
				if (arg) {
					StackItem* item = Stack_pop(argStack);
					if (item->type == Type_FLOAT) {
						float tempVal = *(float*) item->item;
						printf(temp, tempVal);
					} else if (item->type == Type_CHAR){
						char tempVal = *(char*) item->item;
						printf(temp, tempVal);
					} else if (item->type == Type_INT){
						int tempVal = *(int*) item->item;
						printf(temp, tempVal);
					} else if (item->type == Type_STRING){
						char* tempVal = (char*) item->item;
						printf(temp, tempVal);
					} else {
						printf("NaN");
					}
					StackItem_dispose(item);
				} else {
					printf("%s", temp);
					arg = 1;
				}
				free(temp);
				start = i;
			}
		}
	} else {
		errorMessage = "print expects the first argument to be a string";
		longjmp(errBuf, 1);
	}
	StackItem_dispose(format);
	Stack_dispose(argStack);
}

void int_read(Thread* thread, long label) {
	Stack* argStack = Stack_create();
	while (thread->stack->length > label) {
		Stack_push(argStack, Stack_pop(thread->stack));
	}
	if (argStack->length > 0) {
		StackItem* stream = Stack_pop(argStack);
		if (argStack->length > 0) {
			StackItem* spec = Stack_pop(argStack);
			if (spec->type == Type_INT) {
				if (stream->type == Type_STREAM) {
					StackItem* (*read)(int stream, int amt) = *(void**) (&stream + 1);
					Stack_push(thread->stack, read(*(int*) stream->item, *(int*) spec->item));
				} else if (stream->type == Type_FILE_STREAM) {
					StackItem* (*read)(FILE* stream, int amt) = *(void**) (&stream + 1);
					Stack_push(thread->stack, read((FILE*) stream->item, *(int*) spec->item));
				}
			} else if (spec->type == Type_STRING) {
				int start = 0, end = -1;
				char* fData = (char*) spec->item;
				int fLen = strlen(fData);
				int arg = 0;
				for (int i = 0 ; i <= fLen ; i ++) {
					if (fData[i] == '%' || i == fLen) {
						end = i;
						int len = end - start;
						char* temp = calloc(len + 1, sizeof(char));
						for (int i = 0 ; i < len ; i ++) {
							temp[i] = fData[start + i];
						}
						//temp[len] = 0;
						if (arg) {
							char* val = calloc(255, sizeof(char));
							if (stream->type == Type_FILE_STREAM) {
								fscanf((FILE*) stream->item, temp, val);
							} else if (stream->type == Type_STREAM) {
								errorMessage = "formatted reads can only be performed on files and stdin";
								longjmp(errBuf, 1);
							}
							val = realloc(val, (strlen(val) + 1) * sizeof(char));
							Stack_push(thread->stack, newStackItem(Type_STRING, val));
						} else {
							int length = strlen(temp);
							if (length > 0) {
								char* val = calloc(length + 1, sizeof(char));
								strcpy(val, temp);
								Stack_push(thread->stack, newStackItem(Type_STRING, val));
							}
							arg = 1;
						}
						free(temp);
						start = i;
					}
				}
			} else {
				errorMessage = "invalid arguments to function read";
				longjmp(errBuf, 1);
			}
			StackItem_dispose(spec);
		}
		StackItem_dispose(stream);
	} else {
		errorMessage = "invalid arguments to function read";
		longjmp(errBuf, 1);
	}
	Stack_dispose(argStack);
}

void int_set(Thread* thread, long label) {
	StackItem* arg2 = Stack_pop(thread->stack);
	StackItem* arg1 = Stack_pop(thread->stack);
	StackItem* item = Stack_pop(thread->stack);
	if (item->type == Type_ARRAY || item->type == Type_ARRAY_REF) {
		Array* array = item->item;
		if (arg1->type == Type_INT) {
			int index = *(int*) arg1->item;
			if (index >= array->length) {
				array->data = realloc(array->data, (index + 1) * sizeof(StackItem));
				for (; array->length < index + 1 ; array->length ++) {
					array->data[array->length].type = Type_NULL;
					array->data[array->length].item = NULL;
				}
			}
			array->data[index].type = arg2->type;
			array->data[index].item = StackItem_copyValue(arg2);
		}
		free(arg2); // we don't want to dispose of the array here
		StackItem_dispose(arg1);
	} else {
		// Do something else
	}
}

void int_size(Thread* thread, long label) {
	StackItem* item = Stack_pop(thread->stack);
	int* val = malloc(sizeof(int));
	switch (item->type) {
		case Type_ARRAY_REF:
		case Type_ARRAY: {
			*val = ((Array*) item->item)->length;
			break;}
		case Type_STRING: {
			*val = strlen((char*) item->item);
			break;}
	}
	Stack_push(thread->stack, newStackItem(Type_INT, val));
	if (item->type != Type_ARRAY) StackItem_dispose(item);
}

void int_stackSize(Thread* thread, long label) {
	int* val = malloc(sizeof(int));
	*val = thread->stack->length;
	Stack_push(thread->stack, newStackItem(Type_INT, val));
}

void int_stdin(Thread* thread, long label) {
	Stack_push(thread->stack, newStackItem(Type_FILE_STREAM, stdin));
}

void int_toString(Thread* thread, long label) {
	Stack_push(thread->stack
			, newStackItem(Type_STRING
				, StackItem_toString(
					Stack_pop(thread->stack))));
}

void int_write(Thread* thread, long label) {
	Stack* argStack = Stack_create();
	while (thread->stack->length > label) {
		Stack_push(argStack, Stack_pop(thread->stack));
	}
	if (argStack->length > 0) {
		StackItem* stream = Stack_pop(argStack);
		while (argStack->length > 0) {
			StackItem* item = Stack_pop(argStack);
			if (item->type == Type_STRING) {
				if (stream->type == Type_FILE_STREAM) {
					fputs((char*) item->item, (FILE*) stream->item);
				} else if (stream->type == Type_STREAM) {
				}
			}
			StackItem_dispose(item);
		}
		StackItem_dispose(stream);
	} else {
		errorMessage = "invalid arguments to function write";
		longjmp(errBuf, 1);
	}
	Stack_dispose(argStack);
}
