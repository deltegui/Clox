#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "logger.h"


int digitsof(int number) {
    int digits = 0;
    while(number > 0) {
        number = number / 10;
        digits++;
    }
    return digits;
}


void log_fatal(int code, char* message) {
    fprintf(stderr, "%s\n", message);
    exit(code);
}


void log_fatal_line(int code, int line, char* message) {
    size_t message_size = sizeof(message);
    int line_digits = digitsof(line);
    char buffer[message_size + line_digits];
    sprintf(buffer, "[%d] %s", line, message);
    log_fatal(code, buffer);
}