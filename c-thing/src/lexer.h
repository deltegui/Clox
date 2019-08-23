#ifndef LEXER_H
#define LEXER_H

typedef unsigned short token_t;
typedef unsigned long line_t;

//Single character tokens
#define TOKEN_LEFT_PAREN 0
#define TOKEN_RIGHT_PAREN 1
#define TOKEN_LEFT_BRACE 2
#define TOKEN_RIGHT_BRACE 3
#define TOKEN_COMMA 4
#define TOKEN_DOT 5
#define TOKEN_MINUS 6
#define TOKEN_PLUS 7
#define TOKEN_SEMICOLON 8
#define TOKEN_SLASH 9
#define TOKEN_STAR 10

//One or two character tokens
#define TOKEN_BANG 11
#define TOKEN_BANG_EQUAL 12

#define TOKEN_EQUAL 13
#define TOKEN_EQUAL_EQUAL 14

#define TOKEN_GREATER 15
#define TOKEN_GREATER_EQUAL 16

#define TOKEN_LESS 17
#define TOKEN_LESS_EQUAL 18

//Keywords
#define TOKEN_AND 19
#define TOKEN_CLASS 20
#define TOKEN_ELSE 21
#define TOKEN_FALSE 22
#define TOKEN_FUN 23
#define TOKEN_FOR 24
#define TOKEN_IF 25
#define TOKEN_NIL 26
#define TOKEN_OR 27
#define TOKEN_PRINT 28
#define TOKEN_RETURN 29
#define TOKEN_SUPER 30
#define TOKEN_THIS 31
#define TOKEN_TRUE 32
#define TOKEN_VAR 33
#define TOKEN_WHILE 34

//Literals
#define TOKEN_STRING 35
#define TOKEN_NUMBER 36
#define TOKEN_IDENTIFIER 37

typedef struct {
    token_t type;
    char* lexeme;
    char* literal;
    line_t line;
} Token;

typedef struct QueueNode {
    struct QueueNode* next;
    Token* value;
} QueueNode;

typedef struct {
    QueueNode* first;
    QueueNode* last;
    unsigned long size;
} TokenQueue;

/**
 * Passes lox source to tokens
 */
TokenQueue* tokenize_source(char* source);

/**
 * Pulls data from TokenQueue
 */
Token* pull(TokenQueue* queue);

/**
 * Frees all contents on queue (including data)
 */
void destroy_all_queue(TokenQueue* queue);

/**
 * Frees only queue related information (data still reachable)
 */
void destroy_queue(TokenQueue* queue);

/**
 * Prints queue without modifing it
 */
void print_queue(TokenQueue queue);

#endif