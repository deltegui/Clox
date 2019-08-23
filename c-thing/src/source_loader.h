#ifndef SOURCE_LOADER_H
#define SOURCE_LOADER_H

/**
 * Reads Lox source file from source_path.
 * If something bad happens, logs error and
 * stop execution. Always returns assigned
 * memory.
 */
char* read_source(char* source_path);

#endif