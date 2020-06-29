#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sysexits.h"
#include "memory.h"
#include "preproc.h"

static char* read_source_file(const char* file_anme);
static void open_scanner(const char* file_name);
static void push_file_buffer(const char* source);
static void push_scanner(Scanner* scanner);
static void pop_scanner();

typedef struct {
    Scanner** scanner_stack;
    int stack_top;
    int stack_capacity;
    const char** file_buffers;
    int file_buffers_top;
    int file_buffers_capacity;
} Preproc;

Preproc preproc;

void init_preproc(const char* main_file) {
    preproc.scanner_stack = NULL;
    preproc.stack_top = 0;
    preproc.stack_capacity = 0;
    preproc.file_buffers = NULL;
    preproc.file_buffers_top = 0;
    open_scanner(main_file);
}

void free_preproc() {
    while(preproc.stack_top != 0) pop_scanner();
    while(preproc.file_buffers_top != 0) {
        char* last_buffer = (char*)preproc.file_buffers[--preproc.file_buffers_top];
        free(last_buffer);
    }
    FREE_ARRAY(Scanner*, preproc.scanner_stack, preproc.stack_capacity);
    FREE_ARRAY(char*, preproc.file_buffers, preproc.file_buffers_capacity);
}

Token scan_preproc_token() {
    printf("CURRENT stack top: %d\n", preproc.stack_top);
    #define SCAN_TOKEN() scan_token(preproc.scanner_stack[preproc.stack_top-1])
    Token token = SCAN_TOKEN();
    if(token.type == TOKEN_PAD) {
        Token directive = SCAN_TOKEN();
        if(directive.type == TOKEN_INCLUDE) {
            Token file_str = SCAN_TOKEN();
            char* file_name = (char*) malloc(sizeof(char) * (file_str.length-2));
            memcpy(file_name, file_str.start+1, file_str.length-2);
            file_name[file_str.length-2] = '\0';
            printf("String length: %d\n", strlen(file_name));
            open_scanner(file_name);
            free(file_name);
            return SCAN_TOKEN();
        }
    }
    if(token.type == TOKEN_EOF) {
        pop_scanner();
        return SCAN_TOKEN();
    }
    return token;
    #undef SCAN_TOKEN
}

static void open_scanner(const char* file_name) {
    char* file_buffer = read_source_file(file_name);
    push_file_buffer(file_buffer);
    Scanner* scanner = init_scanner(file_buffer);
    push_scanner(scanner);
}

// TODO repeated code in pushes
static void push_file_buffer(const char* source) {
    if(preproc.file_buffers_top >= preproc.file_buffers_capacity) {
        int old_capacity = preproc.file_buffers_capacity;
        preproc.file_buffers_capacity = GROW_CAPACITY(preproc.file_buffers_capacity);
        preproc.file_buffers = GROW_ARRAY(preproc.file_buffers, const char*, old_capacity, preproc.file_buffers_capacity);
    }
    preproc.file_buffers[preproc.file_buffers_top] = source;
    preproc.file_buffers_top++;
}

static void push_scanner(Scanner* scanner) {
    if(preproc.stack_top >= preproc.stack_capacity) {
        int old_capacity = preproc.stack_capacity;
        preproc.stack_capacity = GROW_CAPACITY(preproc.stack_capacity);
        preproc.scanner_stack = GROW_ARRAY(preproc.scanner_stack, Scanner*, old_capacity, preproc.stack_capacity);
    }
    preproc.scanner_stack[preproc.stack_top] = scanner;
    preproc.stack_top++;
}

static void pop_scanner() {
    free_scanner(preproc.scanner_stack[preproc.stack_top-1]);
    preproc.scanner_stack[preproc.stack_top-1] = NULL;
    preproc.stack_top--;
}

/**
 * WARNING: this function can lead to memory leaks if a file cannot
 * be readed.
 * TODO: FIXME MOTHERFUCKER
 */
static char* read_source_file(const char* file_name) {
    printf("FILE: %s\n", file_name);
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