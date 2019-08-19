#include <stdio.h>
#include <sysexits.h>
#include <stdlib.h>

#include "source_loader.h"
#include "logger.h"

char* read_source(char* source_path) {
    FILE* source_file = fopen(source_path, "r");
    if(!source_file) {
        log_fatal(EX_NOINPUT, "Cannot read source file!");
    }
    fseek(source_file, 0, SEEK_END);
    long size = ftell(source_file);
    rewind(source_file);
    if(size <= 0) {
        char* fake = malloc(1);
        return fake;
    }
    char* buffer = (char*) malloc(size);
    if(!buffer) {
        log_fatal(1, "Cannot assign memory to file buffer");
    }
    fread(buffer, 1, size, source_file);
    fclose(source_file);
    return buffer;
}
