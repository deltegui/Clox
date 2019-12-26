#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "values.h"
#include "table.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocate_object(sizeof(type), objectType)

static ObjString* allocate_string(const char* chars, int length, uint32_t hash);
static Obj* allocate_object(size_t size, ObjType type);
static uint32_t hash_string(const char* chars, int length);

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
    table_set(&vm.strings, string, NIL_VALUE());
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