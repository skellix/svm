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
// Standard Funcs
void int_array(Thread* thread, long label);
void int_break(Thread* thread, long label);
void int_call(Thread* thread, long label);
void int_class(Thread* thread, long label);
void int_close(Thread* thread, long label);
void int_continue(Thread* thread, long label);
void int_dumpFrame(Thread* thread, long label);
void int_dumpLocals(Thread* thread, long label);
void int_dumpStack(Thread* thread, long label);
void int_function(Thread* thread, long label);
void int_listClasses(Thread* thread, long label);
void int_listFunctions(Thread* thread, long label);
void int_new(Thread* thread, long label);
void int_open(Thread* thread, long label);
void int_printf(Thread* thread, long label);
void int_read(Thread* thread, long label);
void int_set(Thread* thread, long label);
void int_size(Thread* thread, long label);
void int_stackSize(Thread* thread, long label);
void int_stdin(Thread* thread, long label);
void int_toString(Thread* thread, long label);
void int_write(Thread* thread, long label);

#endif//INTERRUPT_H
