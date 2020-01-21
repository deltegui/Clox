#include "memory.h"
#include "chunk.h"

void* reallocate(void* oldptr, size_t old_count, size_t count) {
	if (count == 0) {
		free(oldptr);
		return NULL;
	}
	return realloc(oldptr, count);
}

void free_object(Obj* object) {
	switch (object->type) {
	case OBJ_FUNCTION: {
		ObjFunction* func = (ObjFunction*)object;
		free_chunk(&func->chunk);
		FREE(ObjFunction, object);
		break;
	}
    case OBJ_STRING: {
		ObjString* string = (ObjString*)object;
		FREE_ARRAY(char, string->chars, string->length + 1);
		FREE(ObjString, object);
		break;
    }
  }
}