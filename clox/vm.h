#ifndef clox_vm_h 
#define clox_vm_h 

#include "chunk.h"
#include "values.h"

#define STACK_MAX 256

typedef struct {
	Chunk* chunk;
	uint8_t* pc;
	Value stack[STACK_MAX];
	Value* stack_top;
} VM;

typedef enum {
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR,
} InterpretResult;

void init_vm();
void free_vm();
InterpretResult interpret_chunk(Chunk* chunk);
InterpretResult interpret(const char* source);

#endif  