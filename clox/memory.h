#ifndef clox_memory_h
#define clox_memory_h

#include <stdlib.h>
#include "object.h"

#define ALLOCATE(type, count) \
    (type*)reallocate(NULL, 0, sizeof(type) * (count))

#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity) * 2);

#define GROW_ARRAY(oldptr, type, old_count, count) \
		(type*) reallocate(oldptr, old_count * sizeof(type), count * sizeof(type))

#define FREE_ARRAY(type, ptr, count) \
		(type*) reallocate(ptr, count * sizeof(type), 0)

#define FREE(type, pointer) \
    reallocate(pointer, sizeof(type), 0)

void* reallocate(void* oldptr, size_t old_count, size_t count);
void free_object(Obj* object);

#endif
