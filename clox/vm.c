#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "vm.h"
#include "object.h"
#include "debug.h"
#include "compiler.h"
#include "memory.h"

VM vm;

static InterpretResult run();
static void stack_push(Value value);
static Value stack_pop();
static void stack_reset();
static Value stack_peek(int distance);
static void runtime_error(const char* format, ...);
static void free_objects();
static void free_object(Obj* object);
static void concatenate_str();

void init_vm() {
	stack_reset();
	vm.objects = NULL;
	init_table(&vm.strings);
	init_table(&vm.globals);
}

void free_vm() {
	free_table(&vm.globals);
	free_table(&vm.strings);
	free_objects();
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
#define READ_STRING() AS_STRING(READ_CONSTANT())
#define BINARY_OP(value_type, op) \
	do {\
		if(!IS_NUMBER(stack_peek(0)) || !IS_NUMBER(stack_peek(1))) { \
			runtime_error("Operand must be a number"); \
			return INTERPRET_RUNTIME_ERROR; \
		} \
		double b = AS_NUMBER(stack_pop()); \
		double a = AS_NUMBER(stack_pop()); \
		stack_push(value_type(a op b)); \
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
		case OP_RETURN: return INTERPRET_OK;
		case OP_POP: stack_pop(); break;
		case OP_CONSTANT: {
			Value constant = READ_CONSTANT();
			stack_push(constant);
			break;
		}
		case OP_NIL: stack_push(NIL_VALUE()); break;
		case OP_TRUE: stack_push(BOOL_VALUE(true)); break;
		case OP_FALSE: stack_push(BOOL_VALUE(false)); break;
		case OP_NOT:
			stack_push(BOOL_VALUE(is_falsy(stack_pop())));
			break;
		case OP_LESS: BINARY_OP(BOOL_VALUE, <); break;
		case OP_GREATER: BINARY_OP(BOOL_VALUE, >); break;
		case OP_EQUAL: {
			Value right = stack_pop();
			Value left = stack_pop();
			stack_push(BOOL_VALUE(values_equal(left, right)));
			break;
		}
		case OP_NEGATE: {
			if(!IS_NUMBER(stack_peek(0))) {
				runtime_error("Operand must be a number");
				return INTERPRET_RUNTIME_ERROR;
			}
			stack_push( NUMBER_VALUE( - AS_NUMBER( stack_pop() ) ) );
			break;
		}
		case OP_ADD: {
			if(IS_NUMBER(stack_peek(0)) && IS_NUMBER(stack_peek(1))) {
				double b = AS_NUMBER(stack_pop());
				double a = AS_NUMBER(stack_pop());
				stack_push(NUMBER_VALUE(a + b));
			} else if(IS_STRING(stack_peek(0)) && IS_STRING(stack_peek(1))) {
				concatenate_str();
			} else {
				runtime_error("Operand must be two numbers or two strings");
				return INTERPRET_RUNTIME_ERROR;
			}
			break;
		}
		case OP_SUBSTRACT: BINARY_OP(NUMBER_VALUE, -); break;
		case OP_MULTIPLY: BINARY_OP(NUMBER_VALUE, *); break;
		case OP_DIVIDE: BINARY_OP(NUMBER_VALUE, /); break;
		case OP_PRINT: {
			print_value(stack_pop());
			printf("\n");
			break;
		}
		case OP_DEFINE_GLOBAL: {
			ObjString* name = READ_STRING();
			table_set(&vm.globals, name, stack_peek(0));
			stack_pop(); // Ensure garbage collector can access the value if is triggered here.
			break;
		}
		case OP_GET_GLOBAL: {
			ObjString* name = READ_STRING();
			Value value;
			if(!table_get(&vm.globals, name, &value)) {
				runtime_error("Undefined global: %s", name->chars);
				return INTERPRET_RUNTIME_ERROR;
			}
			stack_push(value);
			break;
		}
		case OP_SET_GLOBAL: {
			ObjString* name = READ_STRING();
			if(table_set(&vm.globals, name, stack_peek(0))) {
				table_delete(&vm.globals, name);
				runtime_error("Undefined global: %s", name->chars);
				return INTERPRET_RUNTIME_ERROR;
			}
			break;
		}
		case OP_GET_LOCAL: {
			uint8_t stack_index = READ_BYTE();
			stack_push(vm.stack[stack_index]);
			break;
		}
		case OP_SET_LOCAL: {
			uint8_t stack_index = READ_BYTE();
			vm.stack[stack_index] = stack_peek(0);
			break;
		}
		}
	}
#undef BINARY_OP
#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
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

static Value stack_peek(int distance) {
	return vm.stack_top[-1 - distance];
}

static void runtime_error(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  size_t instruction = vm.pc - vm.chunk->code;
  int line = vm.chunk->lines[instruction];
  fprintf(stderr, "[line %d] in script\n", line);

  stack_reset();
}

static void concatenate_str() {
	ObjString* b = AS_STRING(stack_pop());
	ObjString* a = AS_STRING(stack_pop());

	int length = b->length + a->length;
	char* chars = ALLOCATE(char, length + 1);
	memcpy(chars, a->chars, a->length);
	memcpy(chars + a->length, b->chars, b->length);
	chars[length] = '\0';

	ObjString* result = take_string(chars, length);
	stack_push(OBJ_VALUE(result));
}

static void free_objects() {
	Obj* current = vm.objects;
	while(current != NULL) {
		Obj* next = current->next;
		free_object(current);
		current = next;
	}
}

static void free_object(Obj* object) {
	switch (object->type) {
    case OBJ_STRING: {
      ObjString* string = (ObjString*)object;
      FREE_ARRAY(char, string->chars, string->length + 1);
      FREE(ObjString, object);
      break;
    }
  }
}