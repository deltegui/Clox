#include <stdio.h>
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
	FREE_ARRAY(array->values, Value, array->capacity);
	init_valuearray(array);
}

void print_value(Value value) {
	switch(value.type) {
	case VAL_BOOL: printf("%s", AS_BOOL(value) ? "true" : "false"); break;
	case VAL_NUMBER: printf("%g", AS_NUMBER(value)); break;
	case VAL_NIL: printf("nil"); break;
	}
}