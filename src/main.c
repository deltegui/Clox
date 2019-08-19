#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>

#include "source_loader.h"
#include "logger.h"

void run(char* source) {
    printf("%s", source);
}

void runFile(char* path) {
    char* content = read_source(path);
    run(content);
    free(content);
}

void runPrompt() {
    const size_t SIZE = 256;
    for(;;) {
        char* buffer = (char*) malloc(SIZE);
        size_t buffer_size = SIZE;
        printf("(lox) ~> ");
        getline(&buffer, &buffer_size, stdin);
        printf("%s", buffer);
        free(buffer);
    }
}

int main(int argc, char** argv) {
    if(argc > 2) {
        log_fatal(EX_USAGE, "Usage: 'loxc [source]'");
    }
    if(argc == 2) {
        runFile(argv[1]);
        return 0;
    }
    if(argc == 1) {
        runPrompt();
    }
    return 0;
}
