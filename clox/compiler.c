#include <stdio.h>
#include <stdlib.h>
#include "compiler.h"
#include "scanner.h"
#include "object.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

typedef struct {
	Token current;
	Token previous;
	bool had_error;
	bool panic_mode;
} Parser;

typedef enum {
	PREC_NONE,
	PREC_ASSIGNMENT,  // =
	PREC_OR,          // or
	PREC_AND,         // and
	PREC_EQUALITY,    // == !=
	PREC_COMPARISON,  // < > <= >=
	PREC_TERM,        // + -
	PREC_FACTOR,      // * /
	PREC_UNARY,       // ! -
	PREC_CALL,        // . () []
	PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(bool can_assign);

typedef struct {
	ParseFn prefix;
	ParseFn infix;
	Precedence precedence;
} ParseRule;

static void expression();
static void statement();
static void print_stmt();
static void expr_stmt();
static void declaration();

static void grouping(bool can_assign);
static void binary(bool can_assign);
static void unary(bool can_assign);
static void number(bool can_assign);
static void literal(bool can_assign);
static void string(bool can_assign);
static void variable(bool can_assign);
static void named_variable(Token name, bool can_assign);

static bool match(TokenType type);
static bool check(TokenType type);
static void syncrhonize();
static void advance();
static void var_declaration();
static uint8_t parse_variable(const char* error_msg);
static uint8_t identifier_constant(Token* identifier);
static void define_variable(uint8_t global);

ParseRule rules[] = {
  { grouping, NULL,    PREC_NONE },       // TOKEN_LEFT_PAREN
  { NULL,     NULL,    PREC_NONE },       // TOKEN_RIGHT_PAREN
  { NULL,     NULL,    PREC_NONE },       // TOKEN_LEFT_BRACE
  { NULL,     NULL,    PREC_NONE },       // TOKEN_RIGHT_BRACE
  { NULL,     NULL,    PREC_NONE },       // TOKEN_COMMA
  { NULL,     NULL,    PREC_NONE },       // TOKEN_DOT
  { unary,    binary,  PREC_TERM },       // TOKEN_MINUS
  { NULL,     binary,  PREC_TERM },       // TOKEN_PLUS
  { NULL,     NULL,    PREC_NONE },       // TOKEN_SEMICOLON
  { NULL,     binary,  PREC_FACTOR },     // TOKEN_SLASH
  { NULL,     binary,  PREC_FACTOR },     // TOKEN_STAR
  { unary,    NULL,    PREC_NONE },       // TOKEN_BANG
  { NULL,     binary,  PREC_EQUALITY },   // TOKEN_BANG_EQUAL
  { NULL,     NULL,    PREC_NONE },       // TOKEN_EQUAL
  { NULL,     binary,  PREC_EQUALITY },   // TOKEN_EQUAL_EQUAL
  { NULL,     binary,  PREC_COMPARISON }, // TOKEN_GREATER
  { NULL,     binary,  PREC_COMPARISON }, // TOKEN_GREATER_EQUAL
  { NULL,     binary,  PREC_COMPARISON }, // TOKEN_LESS
  { NULL,     binary,  PREC_COMPARISON }, // TOKEN_LESS_EQUAL
  { variable, NULL,    PREC_NONE },       // TOKEN_IDENTIFIER
  { string,   NULL,    PREC_NONE },       // TOKEN_STRING
  { number,   NULL,    PREC_NONE },       // TOKEN_NUMBER
  { NULL,     NULL,    PREC_NONE },       // TOKEN_AND
  { NULL,     NULL,    PREC_NONE },       // TOKEN_CLASS
  { NULL,     NULL,    PREC_NONE },       // TOKEN_ELSE
  { literal,  NULL,    PREC_NONE },       // TOKEN_FALSE
  { NULL,     NULL,    PREC_NONE },       // TOKEN_FOR
  { NULL,     NULL,    PREC_NONE },       // TOKEN_FUN
  { NULL,     NULL,    PREC_NONE },       // TOKEN_IF
  { literal,  NULL,    PREC_NONE },       // TOKEN_NIL
  { NULL,     NULL,    PREC_NONE },       // TOKEN_OR
  { NULL,     NULL,    PREC_NONE },       // TOKEN_PRINT
  { NULL,     NULL,    PREC_NONE },       // TOKEN_RETURN
  { NULL,     NULL,    PREC_NONE },       // TOKEN_SUPER
  { NULL,     NULL,    PREC_NONE },       // TOKEN_THIS
  { literal,  NULL,    PREC_NONE },       // TOKEN_TRUE
  { NULL,     NULL,    PREC_NONE },       // TOKEN_VAR
  { NULL,     NULL,    PREC_NONE },       // TOKEN_WHILE
  { NULL,     NULL,    PREC_NONE },       // TOKEN_ERROR
  { NULL,     NULL,    PREC_NONE },       // TOKEN_EOF
};

static ParseRule* get_rule(TokenType type) {
	return &rules[type];
}

Parser parser;

Chunk* compiling_chunk;

static Chunk* current_chunk() {
	return compiling_chunk;
}

static void emit_byte(uint8_t byte) {
	write_chunk(current_chunk(), byte, parser.previous.line);
}

static void emit_bytes(uint8_t byte1, uint8_t byte2) {
	emit_byte(byte1);
	emit_byte(byte2);
}

static void error_at(Token* token, const char* message) {
	if (parser.panic_mode) return;
	parser.panic_mode = true;
	fprintf(stderr, "[line %d] Error", token->line);
	if (token->type == TOKEN_EOF) {
		fprintf(stderr, " at end");
	}
	else if (token->type == TOKEN_ERROR) {
		// Nothing.
	}
	else {
		fprintf(stderr, " at '%.*s'", token->length, token->start);
	}
	fprintf(stderr, ": %s\n", message);
	parser.had_error = true;
}

static void syncrhonize() {
	parser.panic_mode = false;
	while(parser.current.type != TOKEN_EOF) {
		if(parser.previous.type == TOKEN_SEMICOLON) return;
		switch(parser.current.type) {
		case TOKEN_PRINT:
		case TOKEN_CLASS:
		case TOKEN_FUN:
		case TOKEN_VAR:
		case TOKEN_FOR:
		case TOKEN_IF:
		case TOKEN_WHILE:
		case TOKEN_RETURN:
			return;
		default: ;// DO NOTHING. STILL PANIC
		}
		advance();
	}
}

static void error_at_current(const char* message) {
	error_at(&parser.current, message);
}

static void error(const char* message) {
	error_at(&parser.previous, message);
}

static void advance() {
	parser.previous = parser.current;
	for (;;) {
		parser.current = scan_token();
#ifdef DEBUG_PRINT_SCAN
		printf("TOKEN %u\n", parser.current.type);
#endif
		if (parser.current.type != TOKEN_ERROR) break;
		error_at_current(parser.current.start);
	}
}

static void consume(TokenType type, const char* message) {
	if (parser.current.type == type) {
		advance();
		return;
	}
	error_at_current(message);
}

static bool match(TokenType type) {
	if(!check(type)) return false;
	advance();
	return true;
}

static bool check(TokenType type) {
	return parser.current.type == type;
}

static void emit_return() {
	emit_byte(OP_RETURN);
}

static uint8_t make_constant(Value value) {
	int constant_index = add_constant(current_chunk(), value);
	if (constant_index > UINT8_MAX) {
		error("Too many constants in one chunk.");
		return 0;
	}
	return (uint8_t)constant_index;
}

static void emit_constant(Value value) {
	emit_bytes(OP_CONSTANT, make_constant(value));
}

static void end_compiler() {
	emit_return();
#ifdef DEBUG_PRINT_CODE
	if (!parser.had_error) {
		disassemble_chunk(current_chunk(), "code");
	}
#endif
}

static void parse_precedence(Precedence precedence) {
	advance();
	ParseFn prefix_rule = get_rule(parser.previous.type)->prefix;
	if (prefix_rule == NULL) {
		error("Expect expression.");
		return;
	}

	bool can_assign = precedence <= PREC_ASSIGNMENT;
  	prefix_rule(can_assign);

	while (precedence <= get_rule(parser.current.type)->precedence) {
		advance();
		ParseFn infixRule = get_rule(parser.previous.type)->infix;
		infixRule(can_assign);
	}
}

static void declaration() {
	if(match(TOKEN_VAR)) {
		var_declaration();
	} else {
		statement();
	}
	if(parser.panic_mode) syncrhonize();
}

static void var_declaration() {
	uint8_t global = parse_variable("Expected variable name");
	if(match(TOKEN_EQUAL)) {
		expression();
	} else {
		emit_byte(OP_NIL);
	}
	consume(TOKEN_SEMICOLON, "Expected ; after variable declaration");
	define_variable(global);
}

static uint8_t parse_variable(const char* error_msg) {
	consume(TOKEN_IDENTIFIER, error_msg);
	return identifier_constant(&parser.previous);
}

static uint8_t identifier_constant(Token* identifier) {
	return make_constant(OBJ_VALUE(copy_string(identifier->start, identifier->length)));
}

static void define_variable(uint8_t global) {
	emit_bytes(OP_DEFINE_GLOBAL, global);
}

static void statement() {
	if(match(TOKEN_PRINT)) {
		print_stmt();
	} else {
		expr_stmt();
	}
}

static void print_stmt() {
	expression();
	consume(TOKEN_SEMICOLON, "Expected ; after value");
	emit_byte(OP_PRINT);
}

static void expr_stmt() {
	expression();
	consume(TOKEN_SEMICOLON, "Expected ; after value");
	emit_byte(OP_POP);
}

static void expression() {
	parse_precedence(PREC_ASSIGNMENT);
}

static void number(bool can_assign) {
	double value = strtod(parser.previous.start, NULL);
	emit_constant(NUMBER_VALUE(value));
}

static void grouping(bool can_assign) {
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expected ')' after expression.");
}

static void unary(bool can_assign) {
	TokenType operatorType = parser.previous.type;
	parse_precedence(PREC_UNARY); //compile operand
	// Emit the operator instruction.
	switch (operatorType) {
	case TOKEN_BANG: emit_byte(OP_NOT); break;
	case TOKEN_MINUS: emit_byte(OP_NEGATE); break;
	default:
		return; // Unreachable.
	}
}

static void binary(bool can_assign) {
	// Remember the operator.
	TokenType operatorType = parser.previous.type;

	// Compile the right operand.
	ParseRule* rule = get_rule(operatorType);
	parse_precedence((Precedence)(rule->precedence + 1));

	// Emit the operator instruction.
	switch (operatorType) {
	case TOKEN_PLUS:  emit_byte(OP_ADD); break;
	case TOKEN_MINUS: emit_byte(OP_SUBSTRACT); break;
	case TOKEN_STAR:  emit_byte(OP_MULTIPLY); break;
	case TOKEN_SLASH: emit_byte(OP_DIVIDE); break;
	case TOKEN_BANG_EQUAL: emit_bytes(OP_EQUAL, OP_NOT); break;
	case TOKEN_EQUAL_EQUAL: emit_byte(OP_EQUAL); break;
	case TOKEN_GREATER: emit_byte(OP_GREATER); break;
	case TOKEN_GREATER_EQUAL: emit_bytes(OP_LESS, OP_NOT); break;
	case TOKEN_LESS: emit_byte(OP_LESS); break;
	case TOKEN_LESS_EQUAL: emit_bytes(OP_GREATER, OP_NOT); break;
	default:
		return; // Unreachable.
	}
}

static void literal(bool can_assign) {
	switch (parser.previous.type) {
	case TOKEN_NIL: emit_byte(OP_NIL); break;
	case TOKEN_TRUE: emit_byte(OP_TRUE); break;
	case TOKEN_FALSE: emit_byte(OP_FALSE); break;
	default:
		return; // Unreachable
	}
}

static void string(bool can_assign) {
	emit_constant(OBJ_VALUE(copy_string(parser.previous.start + 1, parser.previous.length - 2)));
}

static void variable(bool can_assign) {
	named_variable(parser.previous, can_assign);
}

static void named_variable(Token name, bool can_assign) {
	uint8_t arg = identifier_constant(&name);
	if(can_assign && match(TOKEN_EQUAL)) {
		expression();
		emit_bytes(OP_SET_GLOBAL, arg);
	} else {
		emit_bytes(OP_GET_GLOBAL, arg);
	}
}

bool compile(const char* source, Chunk* chunk) {
	init_scanner(source);
	compiling_chunk = chunk;
	parser.had_error = false;
	parser.panic_mode = false;
	advance();
	while(!match(TOKEN_EOF)) {
		declaration();
	}
	end_compiler();
	return !parser.had_error;
}