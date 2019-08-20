#include <stdio.h>
#include "lexer.h"
#include "logger.h"
#include "sysexits.h"

//END OF SOURCE
#define EOS '\0'

typedef struct {
    line_t _current_line;
    char* _source;
    unsigned long _position;
/*
    unsigned long _start_lexeme;
    TokenVector* _tokens
*/
} Source;

Source create_source_from(char* raw_source) {
    Source new_source;
    new_source._current_line = 1;
    new_source._position = 0;
    new_source._source = raw_source;
    return new_source;
}

int is_end(Source source) {
    char current = source._source[source._position];
    return current == EOS;
}

char consume(Source* source) {
    if(is_end(*source)) {
        return EOS;
    }
    char current = source->_source[source->_position];
    source->_position++;
    return current;
}

int is_next_equal(Source source, char character) {
    if(is_end(source)) {
        return 0;
    }
    char next = source._source[++source._position];
    return next == character;
}

void advance(Source* source, unsigned long poisitons) {
    if(is_end(*source)) {
        return;
    }
    source->_position++;
}

TokenVector* tokenize_source(char* raw) {
    Source source = create_source_from(raw);
    while(!is_end(source)) {
        printf("%c", consume(&source));
    }
    return NULL;
}