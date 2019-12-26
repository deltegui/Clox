#include <stdio.h>
#include <string.h>
#include "object.h"
#include "values.h"
#include "memory.h"

void init_valuearray(ValueArray* array) {
	array->values = NULL;
	array->capacity = 0;
	array->size = 0;
}

void write_valuearray(ValueArray* array, Value value) {
	if (array->capacity < array->size + 1) {
		int new_capacity = GROW_CAPACITY(array->capacity);
		array->values = GROW_ARRAY(
			array->values,
			Value,
			array->capacity,
			new_capacity);
		array->capacity = new_capacity;
	}
	array->values[array->size] = value;
	array->size++;
}

void free_valuearray(ValueArray* array) {
	FREE_ARRAY(Value, array->values, array->capacity);
	init_valuearray(array);
}

void print_value(Value value) {
	switch(value.type) {
	case VAL_BOOL: printf("%s", AS_BOOL(value) ? "true" : "false"); break;
	case VAL_NUMBER: printf("%g", AS_NUMBER(value)); break;
	case VAL_NIL: printf("nil"); break;
	case VAL_OBJ: print_object(value); break;
	}
}

bool values_equal(Value left, Value right) {
	if(left.type != right.type) return false;
	switch(left.type) {
	case VAL_BOOL: return AS_BOOL(left) == AS_BOOL(right);
	case VAL_NUMBER: return AS_NUMBER(left) == AS_NUMBER(right);
	case VAL_NIL: return true;
	case VAL_OBJ: return AS_OBJ(left) == AS_OBJ(right);
	}
}

bool is_falsy(Value value) {
	return value.type == VAL_NIL || (IS_BOOL(value) && AS_BOOL(value) == false);
}