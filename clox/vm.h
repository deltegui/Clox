#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "values.h"
#include "table.h"

#define STACK_MAX 256

typedef struct {
	Chunk* chunk;
	uint8_t* pc;
	Value stack[STACK_MAX];
	Value* stack_top;
	struct sObj* objects;
	Table strings; // Interning
	Table globals; // Global variables
} VM;

typedef enum {
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR,
} InterpretResult;

void init_vm();
void free_vm();
InterpretResult interpret(const char* source);

extern VM vm;

#endif