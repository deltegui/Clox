#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "values.h"
#include "chunk.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_STRING(obj) is_obj_type(obj, OBJ_STRING)
#define IS_FUNCTION(value) is_obj_type(value, OBJ_FUNCTION)
#define IS_NATIVE(value) is_obj_type(value, OBJ_NATIVE)
#define IS_CLOSURE(value) is_obj_type(value, OBJ_CLOSURE)
#define IS_CLASS(value) is_obj_type(value, OBJ_CLASS)

#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)
#define AS_FUNCTION(value) ((ObjFunction*)AS_OBJ(value))
#define AS_NATIVE(value) ((ObjNative*)AS_OBJ(value))->function
#define AS_CLOSURE(value) ((ObjClosure*)AS_OBJ(value))
#define AS_CLASS(value) ((ObjClass*)AS_OBJ(value))

typedef enum {
    OBJ_STRING,
    OBJ_FUNCTION,
    OBJ_NATIVE,
    OBJ_CLOSURE,
    OBJ_UPVALUE,
	OBJ_CLASS,
} ObjType;

struct sObj {
    ObjType type;
	bool is_marked;
    struct sObj* next;
};

struct sObjString {
	Obj obj;
	int length;
	char* chars;
	uint32_t hash;
};

typedef struct sUpvalue {
	Obj obj;
	Value* location;
	Value closed;
	struct sUpvalue* next;
} ObjUpvalue;

typedef struct {
	Obj obj;
	int arity;
	Chunk chunk;
	ObjString* name;
	int upvalue_count;
} ObjFunction;

typedef struct {
	Obj obj;
	ObjFunction* function;
	ObjUpvalue** upvalues;
	int upvalue_count;
} ObjClosure;

typedef struct sObjClass {
	Obj obj;
	ObjString* name;
} ObjClass;

typedef Value (*NativeFn)(int arg_count, Value* args);

typedef struct {
  Obj obj;
  NativeFn function;
} ObjNative;

static inline bool is_obj_type(Value value, ObjType type) {
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

ObjString* copy_string(const char* chars, int length);
void print_object(Value value);
ObjString* take_string(const char* chars, int length);

ObjFunction* new_function();
ObjNative* new_native(NativeFn function);
ObjClosure* new_closure(ObjFunction* function);
ObjUpvalue* new_upvalue(Value* slot);
ObjClass* new_class(ObjString* name);

#endif
