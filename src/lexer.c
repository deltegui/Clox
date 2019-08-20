#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "logger.h"
#include "sysexits.h"

//END OF SOURCE
#define EOS '\0'

Token* create_token_with_literal(token_t type, char* lexeme, void* literal, line_t line) {
    Token* token = (Token*) malloc(sizeof(Token));
    if(!token) {
        return NULL;
    }
    token->type = type;
    token->lexeme = lexeme;
    token->literal = literal;
    token->line = line;
    return token;
}

Token* create_token(token_t type, char* lexeme, line_t line) {
    return create_token_with_literal(type, lexeme, NULL, line);
}

TokenQueue* create_queue() {
    TokenQueue* queue = (TokenQueue*) malloc(sizeof(TokenQueue));
    if(queue == NULL) {
        return NULL;
    }
    queue->first = NULL;
    queue->last = NULL;
    queue->size = 0;
    return queue;
}

void push(TokenQueue* queue, Token* element) {
    QueueNode* node = (QueueNode*) malloc(sizeof(QueueNode));
    if(node == NULL) {
        return;
    }
    node->value = element;
    node->next = NULL;
    queue->size++;
    if(queue->first == NULL && queue->last == NULL) {
        queue->first = node;
        queue->last = node;
        return;
    }
    queue->first->next = node;
    queue->first = queue->first->next;
}

Token* pull(TokenQueue* queue) {
    if(queue->last == NULL) {
        return NULL;
    }
    Token* value = queue->last->value;
    QueueNode* to_destroy = queue->last;
    queue->last = queue->last->next;
    free(to_destroy->value);
    free(to_destroy);
    queue->size--;
    return value;
}

void destroy(TokenQueue* queue) {
    while(queue->size > 0) {
        pull(queue);
    }
    free(queue);
}

void print_queue(TokenQueue queue) {
    QueueNode* current = queue.last;
    while(current != NULL) {
        Token* token = current->value;
        printf("TOKEN with TYPE: %d, Lexeme: %s, literal: %s, in line: %ld\n",
            token->type,
            token->lexeme,
            token->literal,
            token->line);
        current = current->next;
    }
}

typedef struct {
    line_t _current_line;
    char* _source;
    unsigned long _position;
    TokenQueue* _tokens;
} Source;

Source create_source_from(char* raw_source) {
    Source new_source;
    new_source._current_line = 1;
    new_source._position = 0;
    new_source._source = raw_source;
    new_source._tokens = create_queue();
    if(!new_source._tokens) {
        free(raw_source);
        log_fatal(EX_SOFTWARE, "No se ha podido crear la cola de tokens");
    }
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

TokenQueue* tokenize_source(char* raw) {
    Source source = create_source_from(raw);
    while(!is_end(source)) {
        printf("%c", consume(&source));
    }
    Token* t = create_token(TOKEN_AND, "and", 232);
    Token* t2 = create_token(TOKEN_VAR, "var", 231);
    push(source._tokens, t);
    push(source._tokens, t2);
    print_queue(*source._tokens);
    destroy(source._tokens);
    return NULL;
}