#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

typedef struct {
	Token name;
	int depth;
} Local;

typedef struct Compiler {
	Local locals[UINT8_COUNT];
	int local_count;
	int scope_depth;
} Compiler;

typedef struct {
	bool is_in_loop;
	int loop_init; // Stores current loop init position. To use with continue.
	// Stores break statement JUMP. To patch for exit. Doing by this way only one break per loop is admited
	int jump_to_exit;
} LoopMetadata;

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

static void error(const char* message);

static void expression();
static void statement();
static void block_stmt();
static void print_stmt();
static void expr_stmt();
static void if_stmt();
static void for_stmt();
static void while_stmt();
static void break_stmt();
static void continue_stmt();
static void declaration();

static int emit_jump(uint8_t op_code);
static void patch_jump(int jump_position);
static void emit_loop(int back_pos);

static void grouping(bool can_assign);
static void binary(bool can_assign);
static void unary(bool can_assign);
static void number(bool can_assign);
static void literal(bool can_assign);
static void string(bool can_assign);
static void variable(bool can_assign);
static void and_(bool can_assign);
static void or_(bool can_assign);
static void named_variable(Token name, bool can_assign);

static bool match(TokenType type);
static bool check(TokenType type);
static void syncrhonize();
static void advance();

static void var_declaration();
static uint8_t parse_variable(const char* error_msg);
static uint8_t identifier_constant(Token* identifier);
static void define_variable(uint8_t global);
static void declare_variable();
static bool identifier_equals(Token* first, Token* second);
static void mark_initialized();
static int resolve_local(Compiler* compiler, Token* name);

static void begin_scope();
static void end_scope();

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
  { NULL,     binary,  PREC_FACTOR },     // TOKEN_PERCENT
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
  { NULL,     and_,    PREC_AND },        // TOKEN_AND
  { NULL,     NULL,    PREC_NONE },       // TOKEN_CLASS
  { NULL,     NULL,    PREC_NONE },       // TOKEN_ELSE
  { literal,  NULL,    PREC_NONE },       // TOKEN_FALSE
  { NULL,     NULL,    PREC_NONE },       // TOKEN_FOR
  { NULL,     NULL,    PREC_NONE },       // TOKEN_FUN
  { NULL,     NULL,    PREC_NONE },       // TOKEN_IF
  { literal,  NULL,    PREC_NONE },       // TOKEN_NIL
  { NULL,     or_,    PREC_OR },          // TOKEN_OR
  { NULL,     NULL,    PREC_NONE },       // TOKEN_PRINT
  { NULL,     NULL,    PREC_NONE },       // TOKEN_RETURN
  { NULL,     NULL,    PREC_NONE },       // TOKEN_SUPER
  { NULL,     NULL,    PREC_NONE },       // TOKEN_THIS
  { literal,  NULL,    PREC_NONE },       // TOKEN_TRUE
  { NULL,     NULL,    PREC_NONE },       // TOKEN_VAR
  { NULL,     NULL,    PREC_NONE },       // TOKEN_WHILE
  { NULL,     NULL,    PREC_NONE },       // TOKEN_BREAK
  { NULL,     NULL,    PREC_NONE },       // TOKEN_CONTINUE
  { NULL,     NULL,    PREC_NONE },       // TOKEN_ERROR
  { NULL,     NULL,    PREC_NONE },       // TOKEN_EOF
};

static ParseRule* get_rule(TokenType type) {
	return &rules[type];
}

Parser parser;

Compiler* current = NULL;

Chunk* compiling_chunk;

LoopMetadata loop_metadata;

static void add_local(Token name) {
	if(current->local_count == UINT8_COUNT) {
		error("Too many local variables in function");
		return;
	}
	Local* local = &current->locals[current->local_count++];
	local->name = name;
	local->depth = -1;
}

static void init_compiler(Compiler* compiler) {
	compiler->local_count = 0;
	compiler->scope_depth = 0;
	current = compiler;
}

static void init_loop_metadata() {
	loop_metadata.is_in_loop = false;
	loop_metadata.loop_init = -1;
	loop_metadata.jump_to_exit = -1;
}

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
		printf("%s\n", get_token_str(parser.current.type));
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

	declare_variable();
	if(current->scope_depth > 0) return 0;
	return identifier_constant(&parser.previous);
}

static uint8_t identifier_constant(Token* identifier) {
	return make_constant(OBJ_VALUE(copy_string(identifier->start, identifier->length)));
}

static void declare_variable() {
	if(current->scope_depth == 0) {
		return;
	}

	Token* name = &parser.previous;

	for(int i = current->local_count - 1; i >= 0; i--) {
		Local* local = &current->locals[i];
		if(local->depth != -1 && local->depth < current->scope_depth) {
			break;
		}
		if(identifier_equals(name, &local->name)) {
			error("Variable with this name already declared in this scope");
		}
	}

	add_local(*name);
}

static bool identifier_equals(Token* first, Token* second) {
	if(first->length != second->length) return false;
	return memcmp(first->start, second->start, first->length) == 0;
}

static void define_variable(uint8_t global) {
	if(current->scope_depth > 0) {
		mark_initialized();
		return;
	};
	emit_bytes(OP_DEFINE_GLOBAL, global);
}

static void mark_initialized() {
	current->locals[current->local_count-1].depth = current->scope_depth;
}

static void statement() {
	if(match(TOKEN_PRINT)) {
		print_stmt();
	} else if(match(TOKEN_LEFT_BRACE)) {
		begin_scope();
		block_stmt();
		end_scope();
	} else if(match(TOKEN_IF)) {
		if_stmt();
	} else if(match(TOKEN_WHILE)) {
		while_stmt();
	} else if(match(TOKEN_FOR)) {
		for_stmt();
	} else if(match(TOKEN_BREAK)) {
		break_stmt();
	} else if(match(TOKEN_CONTINUE)) {
		continue_stmt();
	} else {
		expr_stmt();
	}
}

static void continue_stmt() {
	consume(TOKEN_SEMICOLON, "Expected ; after break");
	if(!loop_metadata.is_in_loop) {
		error("Illegal continue statment. You can only use continue inside loops.");
	}
	emit_loop(loop_metadata.loop_init);
}

static void break_stmt() {
	consume(TOKEN_SEMICOLON, "Expected ; after break");
	if(!loop_metadata.is_in_loop) {
		error("Illegal break statment. You can only use break inside loops.");
	}
	if(loop_metadata.jump_to_exit != -1) {
		error("You can only have one break inside loop");
	}
	loop_metadata.jump_to_exit = emit_jump(OP_JUMP);
}

static void handle_loop_metadata() {
	if(loop_metadata.jump_to_exit != -1) {
		patch_jump(loop_metadata.jump_to_exit);
	}
	init_loop_metadata();
}

static void start_loop_metadata() {
	loop_metadata.is_in_loop = true;
	loop_metadata.loop_init = current_chunk()->size;
}

static void for_stmt() {
	begin_scope();
	start_loop_metadata(); // Enable continue and break statements
	consume(TOKEN_LEFT_PAREN, "Expected ( after for");
	if(match(TOKEN_SEMICOLON)) {
		// NO INITIALIZER
	} else if (match(TOKEN_VAR)) {
		var_declaration();
	} else {
		expr_stmt();
	}

	int loop_back = current_chunk()->size;

	int exit_jump = -1;
	if(!match(TOKEN_SEMICOLON)) {
		expression();
		consume(TOKEN_SEMICOLON, "Expected ';' after loop condition");

		// JUMP out loop
		exit_jump = emit_jump(OP_JUMP_IF_FALSE);
		emit_byte(OP_POP); // clean condition
	}

	if(!match(TOKEN_RIGHT_PAREN)) {
		int body_jump = emit_jump(OP_JUMP);
		int incremental_start = current_chunk()->size;
		expression();
		emit_byte(OP_POP);
		consume(TOKEN_RIGHT_PAREN, "Expected ) after for");

		emit_loop(loop_back);
		loop_back = incremental_start;
		patch_jump(body_jump);
	}

	statement();

	emit_loop(loop_back);
	if(exit_jump != -1) {
		patch_jump(exit_jump);
		emit_byte(OP_POP); // clean condition
	}
	handle_loop_metadata();
	end_scope();
}

static void while_stmt() {
	int loop_start = current_chunk()->size;
	start_loop_metadata(); // Enable continue and break statements
	consume(TOKEN_LEFT_PAREN, "Expected ( after while");
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expected ) after while");
	int exit_pos = emit_jump(OP_JUMP_IF_FALSE);
	emit_byte(OP_POP);
	statement();
	emit_loop(loop_start);
	patch_jump(exit_pos);
	emit_byte(OP_POP);
	handle_loop_metadata();
}

static void emit_loop(int back_pos) {
	emit_byte(OP_LOOP);
	int offset = current_chunk()->size - back_pos + 2;
	if(offset > UINT16_MAX) {
		error("Body to large in loop");
	}
	emit_byte((offset >> 8) & 0xff);
	emit_byte(offset & 0xff);
}

static void if_stmt() {
	consume(TOKEN_LEFT_PAREN, "Expected ( after if");
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expected ) after condition in if");
	int then_jump_pos = emit_jump(OP_JUMP_IF_FALSE);
	emit_byte(OP_POP);
	statement();
	int else_jump_pos = emit_jump(OP_JUMP);
	patch_jump(then_jump_pos);
	emit_byte(OP_POP);
	if(match(TOKEN_ELSE)) {
		statement();
	}
	patch_jump(else_jump_pos);
}

static int emit_jump(uint8_t op_code) {
	emit_byte(op_code);
	emit_byte(0xff);
	emit_byte(0xff);
	return current_chunk()->size - 2;
}

static void patch_jump(int jump_position) {
	Chunk* chunk = current_chunk();
	int jump = chunk->size - jump_position - 2;

	if(jump > UINT16_MAX) {
		error("Too much code to jump over");
	}

	// Write a clean 16 bit integer as jump argument
	chunk->code[jump_position] = (jump >> 8) & 0xff;
	chunk->code[jump_position + 1] = jump & 0xff;
}

static void begin_scope() {
	current->scope_depth++;
}

static void end_scope() {
	current->scope_depth--;
	while(current->local_count > 0 &&
		  current->locals[current->local_count - 1].depth > current->scope_depth) {
		emit_byte(OP_POP);
		current->local_count--;
	}
}

static void block_stmt() {
	while(!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
		declaration();
	}
	consume(TOKEN_RIGHT_BRACE, "Expected } to match { in block");
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
	case TOKEN_PLUS: emit_byte(OP_ADD); break;
	case TOKEN_MINUS: emit_byte(OP_SUBSTRACT); break;
	case TOKEN_STAR: emit_byte(OP_MULTIPLY); break;
	case TOKEN_SLASH: emit_byte(OP_DIVIDE); break;
	case TOKEN_BANG_EQUAL: emit_bytes(OP_EQUAL, OP_NOT); break;
	case TOKEN_EQUAL_EQUAL: emit_byte(OP_EQUAL); break;
	case TOKEN_GREATER: emit_byte(OP_GREATER); break;
	case TOKEN_GREATER_EQUAL: emit_bytes(OP_LESS, OP_NOT); break;
	case TOKEN_LESS: emit_byte(OP_LESS); break;
	case TOKEN_LESS_EQUAL: emit_bytes(OP_GREATER, OP_NOT); break;
	case TOKEN_PERCENT: emit_byte(OP_MODULE); break;
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

static void and_(bool can_assign) {
	// Logical AND is implemented using JUMP.
	// IS NOT THE WAY TO DO IT. JUST FOR LEARNING PURPOSES.
	int jump = emit_jump(OP_JUMP_IF_FALSE);
	emit_byte(OP_POP);
	parse_precedence(PREC_AND);
	patch_jump(jump);
}

static void or_(bool can_assign) {
	// Logical OR is implemented using JUMP.
	// IS NOT THE WAY TO DO IT. JUST FOR LEARNING PURPOSES.
	int else_jump = emit_jump(OP_JUMP_IF_FALSE);
	int jump = emit_jump(OP_JUMP);
	patch_jump(else_jump);
	emit_byte(OP_POP);
	parse_precedence(PREC_AND);
	patch_jump(jump);
}

static void string(bool can_assign) {
	emit_constant(OBJ_VALUE(copy_string(parser.previous.start + 1, parser.previous.length - 2)));
}

static void variable(bool can_assign) {
	named_variable(parser.previous, can_assign);
}

static void named_variable(Token name, bool can_assign) {
	uint8_t set_op, get_op;
	int arg = resolve_local(current, &name);
	if(arg != -1) {
		set_op = OP_SET_LOCAL;
		get_op = OP_GET_LOCAL;
	} else {
		arg = identifier_constant(&name);
		set_op = OP_SET_GLOBAL;
		get_op = OP_GET_GLOBAL;
	}

	if(can_assign && match(TOKEN_EQUAL)) {
		expression();
		emit_bytes(set_op, (uint8_t)arg);
	} else {
		emit_bytes(get_op, (uint8_t)arg);
	}
}

static int resolve_local(Compiler* compiler, Token* name) {
	for(int i = compiler->local_count - 1; i >= 0; i--) {
		Local* local = &compiler->locals[i];
		if(identifier_equals(&local->name, name)) {
			if(local->depth == -1) {
				error("Cannot read local variable in its own initializer");
			}
			return i;
		}
	}
	return -1;
}

bool compile(const char* source, Chunk* chunk) {
	init_scanner(source);
	init_loop_metadata();
	Compiler compiler;
	init_compiler(&compiler);
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