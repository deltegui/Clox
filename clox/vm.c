#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "vm.h"
#include "object.h"
#include "debug.h"
#include "compiler.h"
#include "memory.h"

VM vm;

static InterpretResult run();
static void stack_reset();
static Value stack_peek(int distance);
static void runtime_error(const char* format, ...);
static void concatenate_str();
static void free_objects();
static bool call_value(Value callee, int arg_count);
static bool call(ObjClosure* closure, int arg_count);
static void define_native(const char* name, NativeFn native);
static ObjUpvalue* capture_upvalue(Value* value);
static void close_upvalues(Value* last);

static Value clock_native(int argCount, Value* args) {
 	return NUMBER_VALUE((double)clock() / CLOCKS_PER_SEC);
}

void init_vm() {
	stack_reset();
	vm.objects = NULL;
	vm.open_upvalues = NULL;
	init_table(&vm.strings);
	init_table(&vm.globals);

	vm.gray_capacity = 0;
	vm.gray_count = 0;
	vm.gray_stack = NULL;

	define_native("clock", clock_native);
}

void free_vm() {
	free_table(&vm.globals);
	free_table(&vm.strings);
	free_objects();
	free(vm.gray_stack);
}

InterpretResult interpret(const char* source) {
	ObjFunction* func = compile(source);
	if(func == NULL) {
		return INTERPRET_COMPILE_ERROR;
	}
	stack_push(OBJ_VALUE(func));
	ObjClosure* closure = new_closure(func);
	CallFrame* frame = &vm.frames[vm.frames_count];
	frame->closure = closure;
	frame->pc = func->chunk.code;
	frame->slots = vm.stack;
	stack_pop();
	stack_push(OBJ_VALUE(closure));
	call_value(OBJ_VALUE(closure), 0);
	return run();
}

static InterpretResult run() {
	CallFrame* frame = &vm.frames[vm.frames_count - 1];

#define READ_BYTE() (*frame->pc++)
#define READ_CONSTANT() (frame->closure->function->chunk.constants.values[READ_BYTE()])
#define READ_STRING() AS_STRING(READ_CONSTANT())
#define READ_SHORT() (frame->pc += 2, (uint16_t)((frame->pc[-2] << 8) | frame->pc[-1]))
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
		disassemble_instruction(&frame->closure->function->chunk, (int)(frame->pc - frame->closure->function->chunk.code));
#endif

		uint8_t instruction;
		switch (instruction = READ_BYTE()) {
		case OP_RETURN: {
			Value result = stack_pop();
			close_upvalues(frame->slots);
	        vm.frames_count--;
	        if (vm.frames_count == 0) {
				stack_pop();
				return INTERPRET_OK;
	        }
	        vm.stack_top = frame->slots;
	        stack_push(result);
	        frame = &vm.frames[vm.frames_count - 1];
	        break;
		};
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
		case OP_MODULE: {
			if(!IS_NUMBER(stack_peek(0)) || !IS_NUMBER(stack_peek(1))) {
				runtime_error("Operand must be a number");
				return INTERPRET_RUNTIME_ERROR;
			}
			double b = AS_NUMBER(stack_pop());
			double a = AS_NUMBER(stack_pop());
			stack_push(NUMBER_VALUE(fmod(a, b)));
			break;
		}
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
			uint8_t slot = READ_BYTE();
			stack_push(frame->slots[slot]);
			break;
		}
		case OP_SET_LOCAL: {
			uint8_t slot = READ_BYTE();
			frame->slots[slot] = stack_peek(0);
			break;
		}
		case OP_JUMP_IF_FALSE: {
			uint16_t offset = READ_SHORT();
			if(is_falsy(stack_peek(0))) {
				frame->pc += offset;
			}
			break;
		}
		case OP_JUMP: {
			uint16_t offset = READ_SHORT();
			frame->pc += offset;
			break;
		}
		case OP_LOOP: {
			uint16_t offset = READ_SHORT();
			frame->pc -= offset;
			break;
		}
		case OP_CALL: {
			uint8_t args = READ_BYTE();
			if(!call_value(stack_peek(args), args)) {
				return INTERPRET_RUNTIME_ERROR;
			}
			frame = &vm.frames[vm.frames_count - 1];
			break;
		}
		case OP_CLOSURE: {
			ObjFunction* func = AS_FUNCTION(READ_CONSTANT());
			ObjClosure* closure = new_closure(func);
			stack_push(OBJ_VALUE(closure));
			for(int i = 0; i < closure->upvalue_count; i++) {
				uint8_t is_local = READ_BYTE();
				uint8_t index = READ_BYTE();
				if(is_local) {
					closure->upvalues[i] = capture_upvalue(frame->slots + index);
				} else {
					closure->upvalues[i] = frame->closure->upvalues[index];
				}
			}
			break;
		}
		case OP_GET_UPVALUE: {
			uint8_t index = READ_BYTE();
			stack_push(*frame->closure->upvalues[index]->location);
			break;
		}
		case OP_SET_UPVALUE: {
			uint8_t index = READ_BYTE();
			*frame->closure->upvalues[index]->location = stack_peek(0);
			break;
		}
		case OP_CLOSE_UPVALUE: {
			close_upvalues(vm.stack_top - 1);
			stack_pop();
			break;
		}
		}
	}
#undef BINARY_OP
#undef READ_SHORT
#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
}

static void stack_reset() {
	vm.stack_top = vm.stack;
	vm.frames_count = 0;
}

void stack_push(Value value) {
	*vm.stack_top = value;
	vm.stack_top++;
}

Value stack_pop() {
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

	for (int i = vm.frames_count - 1; i >= 0; i--) {
	    CallFrame* frame = &vm.frames[i];
	    ObjFunction* func = frame->closure->function;
	    // -1 because the IP is sitting on the next instruction to be
	    // executed.
	    size_t instruction = frame->pc - func->chunk.code - 1;
	    fprintf(stderr, "[line %d] in ",
	            func->chunk.lines[instruction]);
	    if (func->name == NULL) {
	    	fprintf(stderr, "script\n");
	    } else {
	    	fprintf(stderr, "%s()\n", func->name->chars);
	    }
	}

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

static bool call_value(Value callee, int arg_count) {
	switch(OBJ_TYPE(callee)) {
	case OBJ_CLOSURE: return call(AS_CLOSURE(callee), arg_count);
	case OBJ_NATIVE: {
		NativeFn native = AS_NATIVE(callee);
		Value result = native(arg_count, vm.stack_top - arg_count);
		vm.stack_top -= arg_count + 1;
		stack_push(result);
		return true;
	}
	default:
		break;
	}
	runtime_error("Can only call functions and classes");
	return false;
}

static bool call(ObjClosure* closure, int arg_count) {
	if (arg_count != closure->function->arity) {
	    runtime_error("Expected %d arguments but got %d.",
	        closure->function->arity, arg_count);
	    return false;
  	}
  	if(vm.frames_count == FRAMES_MAX) {
  		runtime_error("Stack Overflow");
  		return false;
  	}
	CallFrame* frame = &vm.frames[vm.frames_count++];
	frame->closure = closure;
	frame->pc = closure->function->chunk.code;

	frame->slots = vm.stack_top - arg_count - 1;
	return true;
}

static void free_objects() {
	Obj* current = vm.objects;
	while(current != NULL) {
		Obj* next = current->next;
		free_object(current);
		current = next;
	}
}

static void define_native(const char* name, NativeFn native) {
	// Here we stack first and pop to let gc know that we are working
	// with ObjString* and ObjNative*. Soooo it won't delete these pointers.
	stack_push(OBJ_VALUE(copy_string(name, (int)strlen(name))));
	stack_push(OBJ_VALUE(new_native(native)));
	table_set(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
	stack_pop();
	stack_pop();
}

static ObjUpvalue* capture_upvalue(Value* value) {
	ObjUpvalue* prev_upvalue = NULL;
	ObjUpvalue* upvalue = vm.open_upvalues;

	while(upvalue != NULL && upvalue->location > value) {
		prev_upvalue = upvalue;
		upvalue = upvalue->next;
	}

	if(upvalue != NULL && upvalue->location == value) return upvalue;

	ObjUpvalue* created = new_upvalue(value);

	created->next = upvalue;
	if(prev_upvalue == NULL) {
		vm.open_upvalues = created;
	} else {
		prev_upvalue->next = created;
	}

	return created;
}

static void close_upvalues(Value* last) {
	while(vm.open_upvalues != NULL && vm.open_upvalues->location >= last) {
		ObjUpvalue* upvalue = vm.open_upvalues;
		upvalue->closed = *upvalue->location;
		upvalue->location = &upvalue->closed;
		vm.open_upvalues = upvalue->next;
	}
}