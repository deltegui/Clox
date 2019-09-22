#include <stdio.h>
#include "chunk.h"
#include "debug.h"

int main(int argc, char** argv) {
	Chunk chunk;
	init_chunk(&chunk);
	int constant = add_constant(&chunk, 1.2);
	write_chunk(&chunk, OP_CONSTANT, 123);
	write_chunk(&chunk, constant, 123);
	write_chunk(&chunk, OP_RETURN, 123);
#ifdef DEBUG_TRACE_EXECUTION
	disassemble_chunk(&chunk, "example");
#endif
	free_chunk(&chunk);
    return 0;
}
