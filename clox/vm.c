#include <stdio.h>
#include "vm.h"
#include "debug.h"
#include "compiler.h"

VM vm;

static InterpretResult run();
static void stack_push(Value value);
static Value stack_pop();
static void stack_reset();

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
#define BINARY_OP(op) \
	do {\
		double b = stack_pop();\
		double a = stack_pop();\
		stack_push(a op b);\
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
		case OP_NEGATE: stack_push(-stack_pop()); break;
		case OP_ADD: BINARY_OP(+); break;
		case OP_SUBSTRACT: BINARY_OP(-); break;
		case OP_MULTIPLY: BINARY_OP(*); break;
		case OP_DIVIDE: BINARY_OP(/); break;
		}
	}
#undef BINARY_OP
#undef READ_BYTE
#undef READ_CONSTANT
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