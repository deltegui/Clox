#include <stdio.h>
#include "debug.h"

void disassemble_chunk(Chunk* chunk, const char* name) {
	printf("== %s chunk ==\n", name);
	for (int i = 0; i < chunk->size;) {
		i = disassemble_instruction(chunk, i);
	}
}

int disassemble_instruction(Chunk* chunk, int position) {
	printf("%04d %d ", position, chunk->lines[position]);
	uint8_t opcode = chunk->code[position];
	switch (opcode) {
	case OP_RETURN:
		return simple_instruction("OP_RETURN", position);
	case OP_CONSTANT:
		return constant_instruction(chunk, position);
	default: {
		printf("ERROR: UNDEFINED OPCODE: %d\n", opcode);
		return position + 1;
	}
	}
}

int simple_instruction(const char* name, int position) {
	printf("%s\n", name);
	return position + 1;
}

int constant_instruction(Chunk* chunk, int position) {
	uint8_t constant_addr = chunk->code[position + 1];
	printf("OP_CONSTANT %04d '", constant_addr);
	print_value(chunk->constants.values[constant_addr]);
	printf("'\n");
	return position + 2;
}