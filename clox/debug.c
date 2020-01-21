#include <stdio.h>
#include "debug.h"

static int simple_instruction(const char* name, int position);
static int constant_instruction(const char* op_name, Chunk* chunk, int position);
static int byte_instruction(const char* name, Chunk* chunk, int position);
static int jump_instruction(const char* name, Chunk* chunk, int position, int direction);

#define DIR_FORWARD 1
#define DIR_BACKWARDS -1

char* tokens_names[] = {
	"TOKEN_LEFT_PAREN", "TOKEN_RIGHT_PAREN",
	"TOKEN_LEFT_BRACE", "TOKEN_RIGHT_BRACE",
	"TOKEN_COMMA", "TOKEN_DOT", "TOKEN_MINUS", "TOKEN_PLUS",
	"TOKEN_SEMICOLON", "TOKEN_SLASH", "TOKEN_STAR", "TOKEN_PERCENT",

	// One or two character tokens.
	"TOKEN_BANG", "TOKEN_BANG_EQUAL",
	"TOKEN_EQUAL", "TOKEN_EQUAL_EQUAL",
	"TOKEN_GREATER", "TOKEN_GREATER_EQUAL",
	"TOKEN_LESS", "TOKEN_LESS_EQUAL",

	// Literals.
	"TOKEN_IDENTIFIER", "TOKEN_STRING", "TOKEN_NUMBER",

	// Keywords.
	"TOKEN_AND", "TOKEN_CLASS", "TOKEN_ELSE", "TOKEN_FALSE",
	"TOKEN_FOR", "TOKEN_FUN", "TOKEN_IF", "TOKEN_NIL", "TOKEN_OR",
	"TOKEN_PRINT", "TOKEN_RETURN", "TOKEN_SUPER", "TOKEN_THIS",
	"TOKEN_TRUE", "TOKEN_VAR", "TOKEN_WHILE", "TOKEN_BREAK",
	"TOKEN_CONTINUE",

	"TOKEN_ERROR",
	"TOKEN_EOF"
};

char* get_token_str(int token) {
	return tokens_names[token];
}

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
	case OP_NEGATE:
		return simple_instruction("OP_NEGATE", position);
	case OP_ADD:
		return simple_instruction("OP_ADD", position);
	case OP_SUBSTRACT:
		return simple_instruction("OP_SUBTRACT", position);
	case OP_MULTIPLY:
		return simple_instruction("OP_MULTIPLY", position);
	case OP_DIVIDE:
		return simple_instruction("OP_DIVIDE", position);
	case OP_MODULE:
		return simple_instruction("OP_MODULE", position);
	case OP_TRUE:
		return simple_instruction("OP_TRUE", position);
	case OP_FALSE:
		return simple_instruction("OP_FALSE", position);
	case OP_NIL:
		return simple_instruction("OP_NIL", position);
	case OP_NOT:
		return simple_instruction("OP_NOT", position);
	case OP_EQUAL:
		return simple_instruction("OP_EQUAL", position);
	case OP_LESS:
		return simple_instruction("OP_LESS", position);
	case OP_GREATER:
		return simple_instruction("OP_GREATER", position);
	case OP_PRINT:
		return simple_instruction("OP_PRINT", position);
	case OP_POP:
		return simple_instruction("OP_POP", position);
	case OP_DEFINE_GLOBAL:
		return constant_instruction("OP_DEFINE_GLOBAL", chunk, position);
	case OP_GET_GLOBAL:
		return constant_instruction("OP_GET_GLOBAL", chunk, position);
	case OP_SET_GLOBAL:
		return constant_instruction("OP_SET_GLOBAL", chunk, position);
	case OP_GET_LOCAL:
		return byte_instruction("OP_GET_LOCAL", chunk, position);
	case OP_SET_LOCAL:
		return byte_instruction("OP_SET_LOCAL", chunk, position);
	case OP_CONSTANT:
		return constant_instruction("OP_CONSTANT", chunk, position);
	case OP_JUMP:
		return jump_instruction("OP_JUMP", chunk, position, DIR_FORWARD);
	case OP_JUMP_IF_FALSE:
		return jump_instruction("OP_JUMP_IF_FALSE", chunk, position, DIR_FORWARD);
	case OP_LOOP:
		return jump_instruction("OP_LOOP", chunk, position, DIR_BACKWARDS);
	case OP_CALL:
    	return byte_instruction("OP_CALL", chunk, position);
	default: {
		printf("ERROR: UNDEFINED OPCODE: %d\n", opcode);
		return position + 1;
	}
	}
}

static int simple_instruction(const char* name, int position) {
	printf("%s\n", name);
	return position + 1;
}

static int constant_instruction(const char* op_name, Chunk* chunk, int position) {
	uint8_t constant_addr = chunk->code[position + 1];
	printf("%s %04d '", op_name, constant_addr);
	print_value(chunk->constants.values[constant_addr]);
	printf("'\n");
	return position + 2;
}

static int byte_instruction(const char* name, Chunk* chunk, int position) {
	uint8_t slot = chunk->code[position + 1];
  	printf("%-16s %4d\n", name, slot);
  	return position + 2;
}

static int jump_instruction(const char* name, Chunk* chunk, int position, int direction) {
	uint16_t jump = (uint16_t)((chunk->code[position + 1] << 8) | chunk->code[position + 2]);
	printf("%-16s %4d -> %d\n", name, jump, position + 3 + direction * jump);
	return position + 3;
}

#undef DIR_BACKWARDS
#undef DIR_FORWARD