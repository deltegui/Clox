#include "vm.h"

VM vm;

void init_vm() {

}

void free_vm() {

}

InterpretResult interpret(Chunk* chunk) {
	vm.chunk = chunk;
	vm.pc = vm.chunk->code;
	return run();
}

static InterpretResult run() {
#define READ_BYTE() (*vm.pc++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
	for (;;) {
		uint8_t instruction;
		switch (instruction = READ_BYTE()) {
		case OP_RETURN: {
			return INTERPRET_OK;
		}
		case OP_CONSTANT: {
			Value constant = READ_CONSTANT();
			printValue(constant);
			printf("\n");
			break;
		}
		}
	} 
#undef READ_BYTE
#undef READ_CONSTANT
}