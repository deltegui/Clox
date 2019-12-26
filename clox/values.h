#ifndef clox_value_h
#define clox_value_h

#include "common.h"

typedef struct sObj Obj;
typedef struct sObjString ObjString;

typedef enum {
  VAL_BOOL,
  VAL_NIL,
  VAL_NUMBER,
  VAL_OBJ,
} ValueType;

typedef struct {
	ValueType type;
	union {
		bool boolean;
		double number;
		Obj* obj;
	} as;
} Value;

#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define IS_OBJ(value) ((value).type == VAL_OBJ)

#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)
#define AS_OBJ(value) ((value).as.obj)

#define BOOL_VALUE(value) ((Value){ VAL_BOOL, { .boolean = value } })
#define NIL_VALUE(value) ((Value){ VAL_NIL, { .number = 0 } })
#define NUMBER_VALUE(value) ((Value){ VAL_NUMBER, { .number = value } })
#define OBJ_VALUE(object) ((Value){ VAL_OBJ, { .obj = (Obj*)object } })

typedef struct {
	int capacity;
	int size;
	Value* values;
} ValueArray;

void init_valuearray(ValueArray* array);
void write_valuearray(ValueArray* array, Value value);
void free_valuearray(ValueArray* array);
void print_value(Value value);
bool values_equal(Value left, Value right);
bool is_falsy(Value value);

#endif
