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
