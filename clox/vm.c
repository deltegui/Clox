#include <stdio.h>
#include <stdarg.h>
#include "vm.h"
#include "debug.h"
#include "compiler.h"

VM vm;

static InterpretResult run();
static void stack_push(Value value);
static Value stack_pop();
static void stack_reset();
static Value stack_peek(int distance);
static void runtime_error(const char* format, ...);
static bool is_falsy(Value value);
static bool is_equal(Value left, Value right);

void init_vm() {
	stack_reset();
}

void free_vm() {

}

InterpretResult interpret(const char* source) {
	Chunk chunk;
	init_chunk(&chunk);
	if (!compile(source, &chunk)) {
		free_chunk(&chunk);
		return INTERPRET_COMPILE_ERROR;
	}
	vm.chunk = &chunk;
	vm.pc = vm.chunk->code;
	InterpretResult result = run();
	free_chunk(&chunk);
	return result;
}

static InterpretResult run() {
#define READ_BYTE() (*vm.pc++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(value_type, op) \
	do {\
		if(!IS_NUMBER(stack_peek(0)) || !IS_NUMBER(stack_peek(1))) { \
			runtime_error("Operand must be a number"); \
			return INTERPRET_RUNTIME_ERROR; \
		} \
		double b = AS_NUMBER(stack_pop()); \
		double a = AS_NUMBER(stack_pop()); \
		stack_push(value_type(a op b)); \
	} while(false)

	for (;;) {

#ifdef DEBUG_TRACE_EXECUTION
		printf("         ");
		for (Value* val_ptr = vm.stack; val_ptr < vm.stack_top; val_ptr++) {
			printf("[ ");
			print_value(*val_ptr);
			printf(" ]");
		}
		printf("\n");
		disassemble_instruction(vm.chunk, (int)(vm.pc - vm.chunk->code));
#endif

		uint8_t instruction;
		switch (instruction = READ_BYTE()) {
		case OP_RETURN:
			print_value(stack_pop());
			printf("\n");
			return INTERPRET_OK;
		case OP_CONSTANT: {
			Value constant = READ_CONSTANT();
			stack_push(constant);
			break;
		}
		case OP_NIL: stack_push(NIL_VALUE()); break;
		case OP_TRUE: stack_push(BOOL_VALUE(true)); break;
		case OP_FALSE: stack_push(BOOL_VALUE(false)); break;
		case OP_NOT:
			stack_push(BOOL_VALUE(is_falsy(stack_pop())));
			break;
		case OP_LESS: BINARY_OP(BOOL_VALUE, <); break;
		case OP_GREATER: BINARY_OP(BOOL_VALUE, >); break;
		case OP_EQUAL: {
			Value right = stack_pop();
			Value left = stack_pop();
			stack_push(BOOL_VALUE(is_equal(left, right)));
			break;
		}
		case OP_NEGATE: {
			if(!IS_NUMBER(stack_peek(0))) {
				runtime_error("Operand must be a number");
				return INTERPRET_RUNTIME_ERROR;
			}
			stack_push( NUMBER_VALUE( - AS_NUMBER( stack_pop() ) ) );
			break;
		}
		case OP_ADD: BINARY_OP(NUMBER_VALUE, +); break;
		case OP_SUBSTRACT: BINARY_OP(NUMBER_VALUE, -); break;
		case OP_MULTIPLY: BINARY_OP(NUMBER_VALUE, *); break;
		case OP_DIVIDE: BINARY_OP(NUMBER_VALUE, /); break;
		}
	}
#undef BINARY_OP
#undef READ_BYTE
#undef READ_CONSTANT
}

static bool is_falsy(Value value) {
	return value.type == VAL_NIL || (IS_BOOL(value) && AS_BOOL(value) == false);
}

static bool is_equal(Value left, Value right) {
	if(left.type != right.type) return false;
	switch(left.type) {
	case VAL_BOOL: return AS_BOOL(left) == AS_BOOL(right);
	case VAL_NUMBER: return AS_NUMBER(left) == AS_NUMBER(right);
	case VAL_NIL: return true;
	}
}

static void stack_reset() {
	vm.stack_top = vm.stack;
}

static void stack_push(Value value) {
	*vm.stack_top = value;
	vm.stack_top++;
}

static Value stack_pop() {
	vm.stack_top--;
	return *vm.stack_top;
}

static Value stack_peek(int distance) {
	return vm.stack_top[-1 - distance];
}

static void runtime_error(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  size_t instruction = vm.pc - vm.chunk->code;
  int line = vm.chunk->lines[instruction];
  fprintf(stderr, "[line %d] in script\n", line);

  stack_reset();
}