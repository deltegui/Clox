#include "memory.h"
#include "chunk.h"
#include "vm.h"
#include "compiler.h"

#ifdef DEBUG_LOG_GC
#include <stdio.h>
#include "debug.h"
#endif

#define GC_HEAP_GROW_FACTOR 2

void* reallocate(void* oldptr, size_t old_count, size_t count) {
	vm.bytes_allocated += count - old_count;

	if(count > old_count) {
#ifdef DEBUG_STRESS_GC
		collect_garbage();
#endif
		if(vm.bytes_allocated > vm.next_gc) {
			collect_garbage();
		}
	}

	if (count == 0) {
		free(oldptr);
		return NULL;
	}
	return realloc(oldptr, count);
}

void free_object(Obj* object) {
#ifdef DEBUG_LOG_GC
	printf("%p free type %s\n", (void*)object, get_obj_str(object->type));
#endif
	switch (object->type) {
	case OBJ_FUNCTION: {
		ObjFunction* func = (ObjFunction*)object;
		free_chunk(&func->chunk);
		FREE(ObjFunction, object);
		break;
	}
    case OBJ_STRING: {
		ObjString* string = (ObjString*)object;
		FREE_ARRAY(char, string->chars, string->length + 1);
		FREE(ObjString, object);
		break;
    }
    case OBJ_NATIVE: {
    	FREE(ObjNative, object);
    	break;
    }
    case OBJ_CLOSURE: {
    	ObjClosure* closure = (ObjClosure*)object;
    	FREE_ARRAY(ObjUpvalue*, closure->upvalues, closure->upvalue_count);
    	FREE(ObjClosure, object);
    	break;
    }
    case OBJ_UPVALUE: {
    	FREE(ObjUpvalue, object);
    	break;
    }
	case OBJ_CLASS: {
		ObjClass* klass = (ObjClass*)object;
		free_table(&klass->methods);
		FREE(ObjClass, object);
		break;
	}
	case OBJ_INSTANCE: {
		ObjInstance* instance = (ObjInstance*)object;
		free_table(&instance->fields);
		FREE(ObjInstance, object);
		break;
	}
  }
}

void mark_object(Obj* object) {
	if(object == NULL) return;
	if(object->is_marked) return;
#ifdef DEBUG_LOG_GC
	printf("%p mark ", (void*)object);
	print_value(OBJ_VALUE(object));
	printf("\n");
#endif
	object->is_marked = true;
	if (vm.gray_capacity < vm.gray_count + 1) {
		vm.gray_capacity = GROW_CAPACITY(vm.gray_capacity);
		vm.gray_stack = realloc(vm.gray_stack, sizeof(Obj*) * vm.gray_capacity);
	}
	vm.gray_stack[vm.gray_count++] = object;
}

void mark_value(Value value) {
	if(!IS_OBJ(value)) return;
	mark_object(AS_OBJ(value));
}

static void mark_roots() {
	// Stack
	for(Value* slot = vm.stack; slot < vm.stack_top; slot++) {
		mark_value(*slot);
	}

	// Closures
	for (int i = 0; i < vm.frames_count; i++) {
    	mark_object((Obj*)vm.frames[i].closure);
	}

	// Upvalues
	for (ObjUpvalue* upvalue = vm.open_upvalues;
		upvalue != NULL;
		upvalue = upvalue->next) {
		mark_object((Obj*)upvalue);
	}

	mark_table(&vm.globals);
	mark_compiler_roots();
}

static void mark_array(ValueArray* array) {
	for (int i = 0; i < array->size; i++) {
		mark_value(array->values[i]);
	}
}

static void blacken_object(Obj* obj) {
#ifdef DEBUG_LOG_GC
	printf("%p blacken ", (void*)obj);
	print_value(OBJ_VALUE(obj));
	printf("\n");
#endif
	switch(obj->type) {
	case OBJ_CLOSURE: {
		ObjClosure* closure = (ObjClosure*)obj;
		mark_object((Obj*)closure->function);
		for (int i = 0; i < closure->upvalue_count; i++) {
			mark_object((Obj*)closure->upvalues[i]);
		}
		break;
    }
	case OBJ_FUNCTION: {
		ObjFunction* function = (ObjFunction*)obj;
		mark_object((Obj*)function->name);
		mark_array(&function->chunk.constants);
		break;
	}
	case OBJ_UPVALUE:
		mark_value(((ObjUpvalue*)obj)->closed);
		break;
	case OBJ_CLASS: {
		ObjClass* klass = (ObjClass*)obj;
		mark_table(&klass->methods);
		mark_object((Obj*)klass->name);
		break;
	}
	case OBJ_INSTANCE: {
		ObjInstance* instance = (ObjInstance*)obj;
		mark_object((Obj*)instance->klass);
		mark_table(&instance->fields);
		break;
	}
	case OBJ_NATIVE:
	case OBJ_STRING:
		break; // These object havent childs
	};
}

static void trace_references() {
	while(vm.gray_count > 0) {
		Obj* obj = vm.gray_stack[--vm.gray_count];
		blacken_object(obj);
	}
}

static void sweep() {
	Obj* previous = NULL;
	Obj* object = vm.objects;
	while (object != NULL) {
#ifdef DEBUG_LOG_GC
		printf("%p sweep for object: [%s]\n", (void*)object, get_obj_str(object->type));
#endif
		if (object->is_marked) {
#ifdef DEBUG_LOG_GC
			printf("%p sweep object is marked. It'll survive: [%s]\n", (void*)object, get_obj_str(object->type));
#endif
			object->is_marked = false;
			previous = object;
			object = object->next;
		} else {
#ifdef DEBUG_LOG_GC
			printf("%p sweep: object is going to die [%s]\n", (void*)object, get_obj_str(object->type));
#endif
			Obj* unreached = object;

			object = object->next;
			if (previous != NULL) {
				previous->next = object;
			} else {
				vm.objects = object;
			}

			free_object(unreached);
		}
	}
}

void collect_garbage() {
#ifdef DEBUG_LOG_GC
	printf("-- gc begin\n");
	size_t before = vm.bytes_allocated;
#endif
	mark_roots();
	trace_references();
	table_remove_white(&vm.strings);
	sweep();

	vm.next_gc = vm.bytes_allocated * GC_HEAP_GROW_FACTOR;

#ifdef DEBUG_LOG_GC
	printf("COLLECTED: %ld bytes (from %ld to %ld) next at %ld\n",
		before - vm.bytes_allocated,
		before,
		vm.bytes_allocated,
		vm.next_gc);
	printf("-- gc end\n");
#endif
}