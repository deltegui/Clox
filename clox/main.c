#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm.h"
#include "sysexits.h"

void repl();
void run_file(const char* file_name);
char* read_source_file(const char* file_anme);

int main(int argc, char** argv) {
	init_vm();
	if (argc == 1) {
		repl();
	}
	else if (argc == 2) {
		run_file(argv[1]);
	}
	else {
		fprintf(stderr, "Wrong number of parameters: %d\n", argc);
		fprintf(stderr, "Usage: clox [path] to run a file or clox to run REPL\n");
		free_vm();
		exit(EX_USAGE);
	}
	free_vm();
    return 0;
}

void repl() {
#define BUFFER_SIZE 1024
	char line_buffer[BUFFER_SIZE];
	for (;;) {
		printf("(lox) ~> ");
		if (!fgets(line_buffer, BUFFER_SIZE, stdin)) {
			fprintf(stderr, "Error while reading from stdin!\n");
			exit(EX_IOERR);
		}
		interpret(line_buffer);
	}
#undef BUFFER_SIZE
}

void run_file(const char* file_name) {
	char* source = read_source_file(file_name);
	InterpretResult result = interpret(source);
	free(source);
	if (result == INTERPRET_COMPILE_ERROR) exit(EX_DATAERR);
	if (result == INTERPRET_RUNTIME_ERROR) exit(EX_SOFTWARE);
}

char* read_source_file(const char* file_name) {
	FILE* file = fopen(file_name, "rb");
	if (!file) {
		fprintf(stderr, "Cannot open file: %s\n", file_name);
		exit(EX_IOERR);
	}
	fseek(file, 0L, SEEK_END);
	size_t file_size = ftell(file);
	rewind(file);
	char* read_buffer = (char*)malloc(file_size + 1);
	if (!read_buffer) {
		fprintf(stderr, "Cannot assign memory.\n");
		exit(EX_IOERR);
	}
	size_t bytes_read = fread(read_buffer, sizeof(char), file_size, file);
	if (bytes_read < file_size) {
		fprintf(stderr, "Cannot read file %s", file_name);
		fclose(file);
		free(read_buffer);
		exit(EX_IOERR);
	}
	read_buffer[bytes_read] = '\0';
	fclose(file);
	return read_buffer;
}
