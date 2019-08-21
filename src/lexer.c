#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "lexer.h"
#include "logger.h"
#include "sysexits.h"

//END OF SOURCE
#define EOS '\0'

Token* create_token(token_t type, char* lexeme, char* literal, line_t line) {
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

void destroy_token(Token* token) {
    free(token->lexeme);
    if(token->literal) {
        free(token->literal);
    }
    free(token);
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
        destroy_token(token);
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
    char* _source; //YOU MUST NOT FREE THIS SHIT
    unsigned long _position;
    unsigned long _start_latest_lexeme;
    TokenQueue* _tokens; //THIS NEITHER
    unsigned short _errors;
} Source;

Source create_source_from(char* raw_source) {
    Source new_source;
    new_source._current_line = 1;
    new_source._position = 0;
    new_source._errors = 0;
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

int is_digit(char digit) {
    return digit >= '0' && digit <= '9';
}

int is_alpha(char alpha) {
    return (alpha >= 'A' && alpha <= 'Z') || 
           (alpha >= 'a' && alpha <= 'z') ||
           (alpha == '_');
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
    char current = source._source[source._position];
    return current == character;
}

int is_next_digit(Source source) {
    if(is_end(source)) {
        return 0;
    }
    char next = source._source[source._position + 1];
    if(next == EOS) {
        return 0;
    }
    return is_digit(next);
}

void advance(Source* source) {
    if(is_end(*source)) {
        return;
    }
    if(is_current_equal(*source, '\n')) {
        source->_current_line++;
    }
    source->_position++;
}

void advance_until(Source* source, char limit) {
    while(!is_current_equal(*source, limit)) {
        if(is_end(*source)) {
            return;
        }
        advance(source);
    }
}

int is_current_digit(Source source) {
    if(is_end(source)) {
        return 0;
    }
    char current = source._source[source._position];
    return is_digit(current);
}

int is_current_alpha(Source source) {
    if(is_end(source)) {
        return 0;
    }
    char current = source._source[source._position];
    return is_alpha(current);
}

char* copy_source_substr(Source source) {
    int buffer_size = (source._position - source._start_latest_lexeme);
    char* copy = malloc(buffer_size + 1);
    strncpy(copy, &source._source[source._start_latest_lexeme], buffer_size);
    copy[buffer_size] = '\0';
    return copy;
}

void push_token_with_literal(Source* source, token_t token, char* literal) {
    int buffer_size = source->_position - source->_start_latest_lexeme;
    char* lexeme = malloc(buffer_size);
    strncpy(lexeme, &source->_source[source->_start_latest_lexeme], buffer_size);
    lexeme[buffer_size] = '\0';
    push(source->_tokens, create_token(token, lexeme, literal, source->_current_line));
}

void push_token(Source* source, token_t token) {
    push_token_with_literal(source, token, NULL);
}

int is_keyword(Source source) {
    char* buffered = copy_source_substr(source);
    int keyword = 0;
    if(strcmp(buffered, "and") == 0) {
        push_token(&source, TOKEN_AND);
        keyword = 1;
    }
    if(strcmp(buffered, "class") == 0) {
        push_token(&source, TOKEN_CLASS);
        keyword = 1;
    }
    if(strcmp(buffered, "else") == 0) {
        push_token(&source, TOKEN_ELSE);
        keyword = 1;
    }
    if(strcmp(buffered, "false") == 0) {
        push_token(&source, TOKEN_FALSE);
        keyword = 1;
    }
    if(strcmp(buffered, "for") == 0) {
        push_token(&source, TOKEN_FOR);
        keyword = 1;
    }
    if(strcmp(buffered, "fun") == 0) {
        push_token(&source, TOKEN_FUN);
        keyword = 1;
    }
    if(strcmp(buffered, "if") == 0) {
        push_token(&source, TOKEN_IF);
        keyword = 1;
    }
    if(strcmp(buffered, "nil") == 0) {
        push_token(&source, TOKEN_NIL);
        keyword = 1;
    }
    if(strcmp(buffered, "or") == 0) {
        push_token(&source, TOKEN_OR);
        keyword = 1;
    }
    if(strcmp(buffered, "print") == 0) {
        push_token(&source, TOKEN_PRINT);
        keyword = 1;
    }
    if(strcmp(buffered, "return") == 0) {
        push_token(&source, TOKEN_RETURN);
        keyword = 1;
    }
    if(strcmp(buffered, "super") == 0) {
        push_token(&source, TOKEN_SUPER);
        keyword = 1;
    }
    if(strcmp(buffered, "this") == 0) {
        push_token(&source, TOKEN_THIS);
        keyword = 1;
    }
    if(strcmp(buffered, "true") == 0) {
        push_token(&source, TOKEN_TRUE);
        keyword = 1;
    }
    if(strcmp(buffered, "var") == 0) {
        push_token(&source, TOKEN_VAR);
        keyword = 1;
    }
    if(strcmp(buffered, "while") == 0) {
        push_token(&source, TOKEN_WHILE);
        keyword = 1;
    }
    free(buffered);
    return keyword;
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
            advance(source);
            push_token(source, TOKEN_BANG_EQUAL); break;
        }
        push_token(source, TOKEN_BANG); break;
    }
    case '=': {
        if(is_current_equal(*source, '=')) {
            advance(source);
            push_token(source, TOKEN_EQUAL_EQUAL); break;
        }
        push_token(source, TOKEN_EQUAL); break;
    }
    case '>': {
        if(is_current_equal(*source, '=')) {
            advance(source);
            push_token(source, TOKEN_GREATER_EQUAL); break;
        }
        push_token(source, TOKEN_GREATER); break;
    }
    case '<': {
        if(is_current_equal(*source, '=')) {
            advance(source);
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
        if(is_current_equal(*source, '*')) {
            int inside_comment = 1;
            while(inside_comment) {
                advance_until(source, '*');
                advance(source);
                if(is_current_equal(*source, '/')) {
                    advance(source);
                    inside_comment = 0;
                }
            }
            break;
        }
        push_token(source, TOKEN_SLASH); break;
    }
    case '"': {
        advance_until(source, '"');
        if(is_end(*source)) {
            source->_errors = 1;
            log_error(source->_current_line, "Unterminated string");
            break;
        }
        advance(source);
        int buffer_size = (source->_position - source->_start_latest_lexeme - 2); //Without ""
        char* literal = malloc(buffer_size + 1); // But with null terminator
        strncpy(literal, &source->_source[source->_start_latest_lexeme + 1], buffer_size);
        literal[buffer_size] = '\0';
        push_token_with_literal(source, TOKEN_STRING, literal);
        break;
    }
    default: {
        if(is_digit(c)) {
            while(is_current_digit(*source)) {
                advance(source);
            }
            if(is_current_equal(*source, '.') && is_next_digit(*source)) {
                advance(source);
                while(is_current_digit(*source)) {
                    advance(source);
                }   
            }
            push_token_with_literal(source, TOKEN_NUMBER, copy_source_substr(*source));
            break;
        }
        if(is_alpha(c)) {
            while(is_current_alpha(*source)) {
                advance(source);
            }
            if(!is_keyword(*source)) {
                push_token_with_literal(source, TOKEN_IDENTIFIER, copy_source_substr(*source));
            }
            break;
        }
        source->_errors = 1;
        char buffer[24];
        sprintf(buffer, "Unexpected character %c", c);
        log_error(source->_current_line, buffer);
    }
    }
}

TokenQueue* tokenize_source(char* raw) {
    Source source = create_source_from(raw);
    while(!is_end(source)) {
        source._start_latest_lexeme = source._position;
        parse_token(&source);
    }
    if(source._errors) {
        free(source._source);
        destroy_all_queue(source._tokens);
        log_fatal(EX_DATAERR, "There where errors during lexing");
    }
    return source._tokens;
}