#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "values.h"
#include "table.h"
#include "vm.h"
#include "debug.h"

#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocate_object(sizeof(type), objectType)

static ObjString* allocate_string(const char* chars, int length, uint32_t hash);
static Obj* allocate_object(size_t size, ObjType type);
static uint32_t hash_string(const char* chars, int length);
void print_function(ObjFunction* func);

ObjString* copy_string(const char* chars, int length) {
    uint32_t hash = hash_string(chars, length);
    ObjString* interned = table_find_string(&vm.strings, chars, length, hash);
    if(interned != NULL) return interned;

    char* heapChars = ALLOCATE(char, length + 1); // Copy string to heap
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    return allocate_string(heapChars, length, hash);
}

static ObjString* allocate_string(const char* chars, int length, uint32_t hash) {
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = (char *)chars;
    string->hash = hash;
    stack_push(OBJ_VALUE(string)); // GC mark needs to discover our object.
    table_set(&vm.strings, string, NIL_VALUE());
    stack_pop(); // GC its safe now.
    return string;
}

static Obj* allocate_object(size_t size, ObjType type) {
    Obj* object = (Obj*)reallocate(NULL, 0, size);
    object->type = type;
    object->is_marked = false;
    object->next = vm.objects;
    vm.objects = object;
#ifdef DEBUG_LOG_GC
    printf("%p allocate %ld for %s\n", (void*)object, size, get_obj_str(type));
#endif
    return object;
}

void print_object(Value value) {
    switch(OBJ_TYPE(value)) {
    case OBJ_FUNCTION: print_function(AS_FUNCTION(value)); break;
    case OBJ_CLOSURE: print_function(AS_CLOSURE(value)->function); break;
    case OBJ_STRING: printf("%s", AS_CSTRING(value)); break;
    case OBJ_NATIVE: printf("<native code>"); break;
    case OBJ_UPVALUE: printf("upvalue"); break;
    case OBJ_CLASS: printf("Class %s", AS_CLASS(value)->name->chars); break;
    case OBJ_INSTANCE: printf("Instance of class %s [%p]", AS_INSTANCE(value)->klass->name->chars, AS_INSTANCE(value)); break;
    case OBJ_BOUND_METHOD:  print_function(AS_BOUND_METHOD(value)->method->function); break;
    }
}

void print_function(ObjFunction* func) {
    if(func->name == NULL) {
        printf("<GLOBAL>");
        return;
    }
    printf("<fn %s>", func->name->chars);
}

ObjString* take_string(const char* chars, int length) {
    uint32_t hash = hash_string(chars, length);
    ObjString* interned = table_find_string(&vm.strings, chars, length, hash);
    if(interned != NULL) {
        FREE_ARRAY(char, (char*)chars, length + 1);
        return interned;
    }
    return allocate_string(chars, length, hash);
}

static uint32_t hash_string(const char* chars, int length) {
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++) {
        hash ^= chars[i];
        hash *= 16777619;
    }
    return hash;
}

ObjFunction* new_function() {
    ObjFunction* func = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
    func->arity = 0;
    func->upvalue_count = 0;
    init_chunk(&func->chunk);
    func->name = NULL;
    return func;
}

ObjNative* new_native(NativeFn function) {
    ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
    native->function = function;
    return native;
}

ObjClosure* new_closure(ObjFunction* function) {
    ObjUpvalue** upvalues = ALLOCATE(ObjUpvalue*, function->upvalue_count);
    for(int i = 0; i < function->upvalue_count; i++) {
        upvalues[i] = NULL;
    }
    ObjClosure* closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
    closure->function = function;
    closure->upvalues = upvalues;
    closure->upvalue_count = function->upvalue_count;
    return closure;
}

ObjUpvalue* new_upvalue(Value* slot) {
    ObjUpvalue* upvalue = ALLOCATE_OBJ(ObjUpvalue, OBJ_UPVALUE);
    upvalue->location = slot;
    upvalue->next = NULL;
    upvalue->closed = NIL_VALUE();
    return upvalue;
}

ObjClass* new_class(ObjString* name) {
    ObjClass* klass = ALLOCATE_OBJ(ObjClass, OBJ_CLASS);
    klass->name = name;
    init_table(&klass->methods);
    return klass;
}

ObjInstance* new_instance(ObjClass* klass) {
    ObjInstance* instance = ALLOCATE_OBJ(ObjInstance, OBJ_INSTANCE);
    instance->klass = klass;
    init_table(&instance->fields);
    return instance;
}

ObjBoundMethod* new_bound_method(Value receiver, ObjClosure* method) {
    ObjBoundMethod* bound = ALLOCATE_OBJ(ObjBoundMethod, OBJ_BOUND_METHOD);
    bound->receiver = receiver;
    bound->method = method;
    return bound;
}