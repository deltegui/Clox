#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "values.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)
#define IS_STRING(obj) is_obj_type(obj, OBJ_STRING)
#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)

typedef enum {
    OBJ_STRING,
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

static inline bool is_obj_type(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

ObjString* copy_string(const char* chars, int length);
void print_object(Value value);
ObjString* take_string(const char* chars, int length);

#endif