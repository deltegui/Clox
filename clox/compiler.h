#ifndef clox_compiler_h
#define clox_compiler_h

#include "common.h"
#include "chunk.h"
#include "object.h"

ObjFunction* compile(const char* source);
void mark_compiler_roots();

#endif