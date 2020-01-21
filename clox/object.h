#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "values.h"
#include "chunk.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)
#define IS_STRING(obj) is_obj_type(obj, OBJ_STRING)
#define IS_FUNCTION(value) is_obj_type(value, OBJ_FUNCTION)
#define IS_NATIVE(value) is_obj_type(value, OBJ_NATIVE);
#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)
#define AS_FUNCTION(value) ((ObjFunction*)AS_OBJ(value))
#define AS_NATIVE(value) ((ObjNative*)AS_OBJ(value))->function;

typedef enum {
    OBJ_STRING,
    OBJ_FUNCTION,
    OBJ_NATIVE,
} ObjType;

struct sObj {
    ObjType type;
    struct sObj* next;
};

struct sObjString {
	Obj obj;
	int length;
	char* chars;
	uint32_t hash;
};

typedef struct {
	Obj obj;
	int arity;
	Chunk chunk;
	ObjString* name;
} ObjFunction;

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

#endif