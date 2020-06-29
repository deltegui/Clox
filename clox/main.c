#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm.h"
#include "sysexits.h"

void repl();
void run_file(const char* file_name);

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
	InterpretResult result = interpret(file_name);
	if (result == INTERPRET_COMPILE_ERROR) exit(EX_DATAERR);
	if (result == INTERPRET_RUNTIME_ERROR) exit(EX_SOFTWARE);
}


