#include <stdlib.h>
#include "chunk.h"
#include "memory.h"
#include "vm.h"

#define INIT_CHUNK_SIZE 4

void init_chunk(Chunk* chunk) {
	chunk->capacity = 0;
	chunk->size = 0;
	chunk->code = NULL;
	chunk->lines = NULL;
	init_valuearray(&chunk->constants);
}

void write_chunk(Chunk* chunk, uint8_t bytecode, int line) {
	if (chunk->capacity < chunk->size + 1) {
		int new_capacity = GROW_CAPACITY(chunk->capacity);
		chunk->code = GROW_ARRAY(
			chunk->code,
			uint8_t,
			chunk->capacity,
			new_capacity);
		chunk->lines = GROW_ARRAY(
			chunk->lines,
			int,
			chunk->capacity,
			new_capacity);
		chunk->capacity = new_capacity;
	}
	chunk->code[chunk->size] = bytecode;
	chunk->lines[chunk->size] = line;
	chunk->size++;
}

void free_chunk(Chunk* chunk) {
	FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
	FREE_ARRAY(int, chunk->lines, chunk->capacity);
	free_valuearray(&chunk->constants);
	init_chunk(chunk);
}

int add_constant(Chunk* chunk, Value value) {
	stack_push(value); // Save value not to be killed by GC mark
	write_valuearray(&chunk->constants, value);
	stack_pop(); // Now it's safe
	return chunk->constants.size - 1; //index of stored constant
}