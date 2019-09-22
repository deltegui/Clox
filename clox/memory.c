#include "memory.h"

void* reallocate(void* oldptr, size_t old_count, size_t count) {
	if (count == 0) {
		free(oldptr);
		return NULL;
	}
	return realloc(oldptr, count);
}