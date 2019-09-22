#ifndef clox_value_h 
#define clox_value_h 

#include "common.h"

typedef double Value;

typedef struct {
	int capacity;
	int size;
	Value* values;
} ValueArray;

void init_valuearray(ValueArray* array);
void write_valuearray(ValueArray* array, Value value);
void free_valuearray(ValueArray* array);
void print_value(Value value);

#endif 
