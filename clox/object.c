#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "values.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocate_object(sizeof(type), objectType)

static ObjString* allocate_string(const char* chars, int length);
static Obj* allocate_object(size_t size, ObjType type);

ObjString* copy_string(const char* chars, int length) {
  char* heapChars = ALLOCATE(char, length + 1); // Copy string to heap
  memcpy(heapChars, chars, length);
  heapChars[length] = '\0';
  return allocate_string(heapChars, length);
}

static ObjString* allocate_string(const char* chars, int length) {
  ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
  string->length = length;
  string->chars = (char *)chars;
  return string;
}

static Obj* allocate_object(size_t size, ObjType type) {
  Obj* object = (Obj*)reallocate(NULL, 0, size);
  object->type = type;
  object->next = vm.objects;
  vm.objects = object;
  return object;
}

void print_object(Value value) {
  switch(OBJ_TYPE(value)) {
  case OBJ_STRING: printf("%s", AS_CSTRING(value)); break;
  }
}

ObjString* take_string(const char* chars, int length) {
  return allocate_string(chars, length);
}