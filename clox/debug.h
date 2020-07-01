#ifndef clox_debug_h
#define clox_debug_h

#include "common.h"
#include "chunk.h"

char* get_token_str(int token);
char* get_obj_str(int obj_type);
void disassemble_chunk(Chunk* chunk, const char* name);
int disassemble_instruction(Chunk* chunk, int position);

#endif