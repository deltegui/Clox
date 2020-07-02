#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "values.h"
#include "table.h"
#include "object.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct {
	ObjClosure* closure;
	uint8_t* pc;
	Value* slots;
} CallFrame;

typedef struct {
	CallFrame frames[FRAMES_MAX];
	int frames_count;

	Value stack[STACK_MAX];
	Value* stack_top;

	Obj* objects;
	ObjUpvalue* open_upvalues;

	Table strings; // Interning
	Table globals; // Global variables

	// GC gray nodes
	int gray_capacity;
	int gray_count;
	Obj** gray_stack;
} VM;

typedef enum {
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR,
} InterpretResult;

void init_vm();
void free_vm();
void stack_push(Value value);
Value stack_pop();
InterpretResult interpret(const char* source);

extern VM vm;

#endif