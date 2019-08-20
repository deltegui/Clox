#ifndef LEXER_H
#define LEXER_H

typedef char* token_t;
typedef unsigned long line_t;

//Single character tokens
#define TOKEN_LEFT_PAREN "("
#define TOKEN_RIGHT_PAREN ")"
#define TOKEN_LEFT_BRACE "{"
#define TOKEN_RIGHT_BRACE "}"
#define TOKEN_COMMA ","
#define TOKEN_DOT "."
#define TOKEN_MINUS "-"
#define TOKEN_PLUS "+"
#define TOKEN_SEMICOLON ";"
#define TOKEN_SLASH "/"
#define TOKEN_STAR "*"

//One or two character tokens
#define TOKEN_BANG "!"
#define TOKEN_BANG_EQUAL "!="

#define TOKEN_EQUAL "="
#define TOKEN_EQUAL_EQUAL "=="

#define TOKEN_GREATER ">"
#define TOKEN_GREATER_EQUAL ">="

#define TOKEN_LESS "<"
#define TOKEN_LESS_EQUAL "<="

//Keywords
#define TOKEN_AND "and"
#define TOKEN_CLASS "class"
#define TOKEN_ELSE "else"
#define TOKEN_FALSE "false"
#define TOKEN_FUN "fun"
#define TOKEN_FOR "for"
#define TOKEN_IF "if"
#define TOKEN_NIL "nil"
#define TOKEN_OR "or"
#define TOKEN_PRINT "print"
#define TOKEN_RETURN "return"
#define TOKEN_SUPER "super"
#define TOKEN_THIS "this"
#define TOKEN_TRUE "true"
#define TOKEN_VAR "var"
#define TOKEN_WHILE "while"

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