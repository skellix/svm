#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdio.h>
#include <setjmp.h>
#include <stdarg.h>
#include <string.h>
#include "gc.h"
#include "bytecode.h"
#include "stack.h"
#include "error.h"
#include "io.h"

// Interrupt Headers
StackItem* int_add(StackItem* lOperand, StackItem* rOperand);
StackItem* int_subtract(StackItem* lOperand, StackItem* rOperand);
StackItem* int_multiply(StackItem* lOperand, StackItem* rOperand);
StackItem* int_divide(StackItem* lOperand, StackItem* rOperand);
StackItem* int_mod(StackItem* lOperand, StackItem* rOperand);
StackItem* int_less(StackItem* lOperand, StackItem* rOperand);
StackItem* int_greater(StackItem* lOperand, StackItem* rOperand);
StackItem* int_logic_not(StackItem* operand);
StackItem* int_jump_test(StackItem* operand);
StackItem* int_equal(StackItem* lOperand, StackItem* rOperand);
StackItem* int_and(StackItem* lOperand, StackItem* rOperand);
StackItem* int_or(StackItem* lOperand, StackItem* rOperand);
StackItem* int_xor(StackItem* lOperand, StackItem* rOperand);
StackItem* int_bit_not(StackItem* operand);
void int_printf(Stack* argStack);
// Interrupt Implementations
StackItem* int_add(StackItem* lOperand, StackItem* rOperand) {
	Type oType = rOperand->type > lOperand->type ? rOperand->type : lOperand->type;
	if (oType == Type_STRING) {
		if (lOperand->type == rOperand->type) {
			char* val = calloc(strlen((char*) lOperand->item) + strlen((char*) rOperand->item) + 1, sizeof(char));
			sprintf(val, "%s%s", (char*) lOperand->item, (char*) rOperand->item);
			StackItem_dispose(lOperand);
			StackItem_dispose(rOperand);
			return newStackItem(Type_STRING, val);
		}
	} else if (oType == Type_CHAR) {
		char* val = malloc(sizeof(char));
		*val = *(char*) lOperand->item + *(char*) rOperand->item;
		StackItem_dispose(lOperand);
		StackItem_dispose(rOperand);
		return newStackItem(oType, val);
	} else if (oType == Type_INT) {
		int* val = malloc(sizeof(int));
		*val = *(int*) lOperand->item + *(int*) rOperand->item;
		StackItem_dispose(lOperand);
		StackItem_dispose(rOperand);
		return newStackItem(oType, val);
	} else if (oType == Type_FLOAT) {
		float* val = malloc(sizeof(float));
		//fprintf(stderr, "casting int to float is not yet supported on this system!!!\n");
		//exit(1);
		//*
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
		return newStackItem(oType, val);
	}
	return newStackItem(Type_NULL, NULL);
}
StackItem* int_subtract(StackItem* lOperand, StackItem* rOperand) {
	Type oType = rOperand->type > lOperand->type ? rOperand->type : lOperand->type;
	if (oType == Type_STRING) {
		fprintf(stderr, "strings types can't be used in math!!!\n");
		exit(1);
	}
	if (oType == Type_CHAR) {
		char* val = malloc(sizeof(char));
		*val = *(char*) lOperand->item - *(char*) rOperand->item;
		StackItem_dispose(lOperand);
		StackItem_dispose(rOperand);
		return newStackItem(oType, val);
	} else if (oType == Type_INT) {
		int* val = malloc(sizeof(int));
		*val = *(int*) lOperand->item - *(int*) rOperand->item;
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
		*val = lo - ro;
		StackItem_dispose(lOperand);
		StackItem_dispose(rOperand);
		return newStackItem(oType, val);
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
void int_printf(Stack* argStack) {
	//printf("calling int_printf\n");
	StackItem* format = Stack_pop(argStack);
	//printf("format = '%s'\n", format->item);
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
				//snprintf(fData + start1, len, "%s", temp);
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
					printf(temp);
					arg = 1;
				}
				free(temp);
				start = i;
			}
		}
	} else {
		fprintf(stderr, "Error:print expects the first argument to be a string!!!");
		exit(0);
	}
	StackItem_dispose(format);
}

#endif // INTERRUPT_H
