#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
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

Token* create_token(token_t type, line_t line) {
    return create_token_with_literal(type, (char*)type, NULL, line);
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
    free(to_destroy);
    queue->size--;
    return value;
}

void destroy_all_queue(TokenQueue* queue) {
    while(queue->size > 0) {
        Token* token = pull(queue);
        free(token);
    }
    free(queue);
}

void destroy_queue(TokenQueue* queue) {
    while(queue->size > 0) {
        pull(queue);
    }
    free(queue);
}

void print_queue(TokenQueue queue) {
    QueueNode* current = queue.last;
    while(current != NULL) {
        Token* token = current->value;
        printf("TOKEN with TYPE: %s, Lexeme: %s, literal: %s, in line: %ld\n",
            token->type,
            token->lexeme,
            token->literal,
            token->line);
        current = current->next;
    }
}

typedef struct {
    line_t _current_line;
    char* _source; //YOU MUST NOT FREE THIS SHIT
    unsigned long _position;
    TokenQueue* _tokens; //THIS NEITHER
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

int is_current_equal(Source source, char character) {
    if(is_end(source)) {
        return 0;
    }
    char next = source._source[source._position];
    return next == character;
}

void advance(Source* source, unsigned long poisitons) {
    if(is_end(*source)) {
        return;
    }
    source->_position++;
}

void advance_until(Source* source, char limit) {
    while(!is_current_equal(*source, limit)) {
        advance(source, 1);
    }
}

void push_token(Source* source, token_t token) {
    push(source->_tokens, create_token(token, source->_current_line));
}

void parse_token(Source* source) {
    char c = consume(source);
    switch(c) {
    case '(': push_token(source, TOKEN_LEFT_PAREN); break;
    case ')': push_token(source, TOKEN_RIGHT_PAREN); break;
    case '{': push_token(source, TOKEN_LEFT_BRACE); break;
    case '}': push_token(source, TOKEN_RIGHT_BRACE); break;
    case ',': push_token(source, TOKEN_COMMA); break;
    case '.': push_token(source, TOKEN_DOT); break;
    case '-': push_token(source, TOKEN_MINUS); break;
    case '+': push_token(source, TOKEN_PLUS); break;
    case ';': push_token(source, TOKEN_SEMICOLON); break;
    case '*': push_token(source, TOKEN_STAR); break;
    case '!': {
        if(is_current_equal(*source, '=')) {
            push_token(source, TOKEN_BANG_EQUAL); break;
        }
        push_token(source, TOKEN_BANG); break;
    }
    case '=': {
        if(is_current_equal(*source, '=')) {
            push_token(source, TOKEN_EQUAL_EQUAL); break;
        }
        push_token(source, TOKEN_EQUAL); break;
    }
    case '>': {
        if(is_current_equal(*source, '=')) {
            push_token(source, TOKEN_GREATER_EQUAL); break;
        }
        push_token(source, TOKEN_GREATER); break;
    }
    case '<': {
        if(is_current_equal(*source, '=')) {
            push_token(source, TOKEN_LESS_EQUAL); break;
        }
        push_token(source, TOKEN_LESS); break;
    }
    case '\n': source->_current_line++; break;
    case ' ':
    case '\r':
    case '\t':
        break;
    case '/': {
        if(is_current_equal(*source, '/')) {
            advance_until(source, '\n');
            break; 
        }
        push_token(source, TOKEN_SLASH); break;
    }
    default: {
        char buffer[24];
        sprintf(buffer, "Unexpected character %c", c);
        log_error(source->_current_line, buffer);
    }
    }
}

TokenQueue* tokenize_source(char* raw) {
    Source source = create_source_from(raw);
    while(!is_end(source)) {
        parse_token(&source);
    }
    return source._tokens;
}