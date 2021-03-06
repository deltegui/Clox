#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "values.h"

typedef enum {
	OP_CONSTANT,
	OP_RETURN,
	OP_NEGATE,
	OP_ADD,
	OP_SUBSTRACT,
	OP_MULTIPLY,
	OP_DIVIDE,
	OP_MODULE,
	OP_NIL,
	OP_TRUE,
	OP_FALSE,
	OP_NOT,
	OP_EQUAL,
	OP_GREATER,
	OP_LESS,
	OP_PRINT,
	OP_POP,
	OP_DEFINE_GLOBAL,
	OP_GET_GLOBAL,
	OP_SET_GLOBAL,
	OP_GET_LOCAL,
	OP_SET_LOCAL,
	OP_SET_UPVALUE,
	OP_GET_UPVALUE,
	OP_CLOSE_UPVALUE,
	OP_JUMP_IF_FALSE,
	OP_JUMP,
	OP_LOOP,
	OP_CALL,
	OP_CLOSURE,
	OP_CLASS,
	OP_GET_PROPERTY,
	OP_SET_PROPERTY,
	OP_METHOD,
	OP_INVOKE,
	OP_INHERIT,
	OP_GET_SUPER,
	OP_SUPER_INVOKE,
} OpCodes;

typedef struct {
	int size;
	int capacity;
	uint8_t* code;
	int* lines;
	ValueArray constants;
} Chunk;

void init_chunk(Chunk* chunk);
void write_chunk(Chunk* chunk, uint8_t bytecode, int line);
void free_chunk(Chunk* chunk);
int add_constant(Chunk* chunk, Value value);

#endif
