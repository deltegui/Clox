#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "scanner.h"

Token make_token(Scanner* scanner, TokenType type) {
	Token token;
	token.type = type;
	token.start = scanner->start;
	token.length = (int)(scanner->current - scanner->start);
	token.line = scanner->line;
	return token;
}

Token error_token(Scanner* scanner, const char* error) {
	Token token;
	token.type = TOKEN_ERROR;
	token.start = error;
	token.length = (int)strlen(error);
	token.line = scanner->line;
	return token;
}

static bool is_at_end(Scanner* scanner) {
	return *scanner->current == '\0';
}

static char peek(Scanner* scanner) {
	return *scanner->current;
}

static char peek_next(Scanner* scanner) {
	if (is_at_end(scanner)) return '\0';
	return *(scanner->current++);
}

static char advance(Scanner* scanner) {
	char c = *scanner->current;
	scanner->current++;
	return c;
}

static bool match(Scanner* scanner, char expected) {
	if (is_at_end(scanner)) return false;
	if (expected != *(scanner->current)) return false;
	scanner->current++;
	return true;
}

static bool match_next(Scanner* scanner, char expected) {
	if(is_at_end(scanner)) return false;
	return *(scanner->current+1) == expected;
}

static void skip_whitespace(Scanner* scanner) {
#define CONSUME_UNTIL(character) while(peek(scanner) != character && !is_at_end(scanner)) advance(scanner)
	for (;;) {
		char c = peek(scanner);
		switch (c) {
		case '\n':
			scanner->line++;
		case ' ':
		case '\r':
		case '\t':
			advance(scanner);
			break;
		case '/':
			if(match_next(scanner, '/')) {
				CONSUME_UNTIL('\n');
			} else {
				return;
			}
			break;
		default:
			return;
		}
	}
#undef CONSUME_UNTIL
}

static Token scan_string(Scanner* scanner) {
	while (peek(scanner) != '"' && !is_at_end(scanner)) {
		if (peek(scanner) == '\n') scanner->line++;
		advance(scanner);
	}
	if (is_at_end(scanner)) return error_token(scanner, "Unterminated string.");
	advance(scanner); //Consume "
	return make_token(scanner, TOKEN_STRING);
}

static bool is_digit(char c) {
	return c >= '0' && c <= '9';
}

static Token scan_number(Scanner* scanner) {
	while (is_digit(peek(scanner))) advance(scanner);
	if (peek(scanner) == '.' && is_digit(peek_next(scanner))) {
		advance(scanner);
		while (is_digit(peek(scanner))) advance(scanner);
	}
	return make_token(scanner, TOKEN_NUMBER);
}

static bool is_alpha(char c) {
	return (c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z') ||
		c == '_';
}

static TokenType check_keyword(Scanner* scanner, int start, int length, const char* rest, TokenType type) {
	if (scanner->current - scanner->start == start + length &&
		memcmp(scanner->start + start, rest, length) == 0) {
		return type;
	}
	return TOKEN_IDENTIFIER;
}

static TokenType identifier_type(Scanner* scanner) {
	switch (*scanner->start) {
	case 'a': return check_keyword(scanner, 1, 2, "nd", TOKEN_AND);
	case 'b': return check_keyword(scanner, 1, 4, "reak", TOKEN_BREAK);
	case 'c':
		if (scanner->current - scanner->start > 1) {
			switch(scanner->start[1]) {
			case 'l': return check_keyword(scanner, 2, 4, "ass", TOKEN_CLASS);
			case 'o': return check_keyword(scanner, 2, 6, "ntinue", TOKEN_CONTINUE);
			}
		}
		break;
	case 'e': return check_keyword(scanner, 1, 3, "lse", TOKEN_ELSE);
	case 'f':
		if (scanner->current - scanner->start > 1) {
			switch (scanner->start[1]) {
			case 'a': return check_keyword(scanner, 2, 3, "lse", TOKEN_FALSE);
			case 'o': return check_keyword(scanner, 2, 1, "r", TOKEN_FOR);
			case 'u': return check_keyword(scanner, 2, 1, "n", TOKEN_FUN);
			}
		}
		break;
	case 'i':
        if (scanner->current - scanner->start > 1) {
            switch (scanner->start[1]) {
            case 'f': return TOKEN_IF;
            case 'n': return check_keyword(scanner, 2, 5, "clude", TOKEN_INCLUDE);
            }
        }
	case 'n': return check_keyword(scanner, 1, 2, "il", TOKEN_NIL);
	case 'o': return check_keyword(scanner, 1, 1, "r", TOKEN_OR);
	case 'p': return check_keyword(scanner, 1, 4, "rint", TOKEN_PRINT);
	case 'r': return check_keyword(scanner, 1, 5, "eturn", TOKEN_RETURN);
	case 's': return check_keyword(scanner, 1, 4, "uper", TOKEN_SUPER);
	case 't':
		if (scanner->current - scanner->start > 1) {
			switch (scanner->start[1]) {
			case 'h': return check_keyword(scanner, 2, 2, "is", TOKEN_THIS);
			case 'r': return check_keyword(scanner, 2, 2, "ue", TOKEN_TRUE);
			}
		}
		break;
	case 'v': return check_keyword(scanner, 1, 2, "ar", TOKEN_VAR);
	case 'w': return check_keyword(scanner, 1, 4, "hile", TOKEN_WHILE);
	}
	return TOKEN_IDENTIFIER;
}

static Token scan_identifier(Scanner* scanner) {
	while (is_alpha(peek(scanner)) || is_digit(peek(scanner))) advance(scanner);
	return make_token(scanner, identifier_type(scanner));
}

Scanner* init_scanner(const char* source) {
	Scanner* scanner = (Scanner*)malloc(sizeof(Scanner));
	scanner->start = source;
	scanner->current = source;
	scanner->line = 1;
	return scanner;
}

void free_scanner(Scanner* scanner) {
	free(scanner);
}

Token scan_token(Scanner* scanner) {
	skip_whitespace(scanner);
	scanner->start = scanner->current;
	if (is_at_end(scanner)) {
		return make_token(scanner, TOKEN_EOF);
	}
	char c = advance(scanner);
	if (is_alpha(c)) return scan_identifier(scanner);
	if (is_digit(c)) return scan_number(scanner);
	switch (c) {
    case '#': return make_token(scanner, TOKEN_PAD);
	case '(': return make_token(scanner, TOKEN_LEFT_PAREN);
	case ')': return make_token(scanner, TOKEN_RIGHT_PAREN);
	case '{': return make_token(scanner, TOKEN_LEFT_BRACE);
	case '}': return make_token(scanner, TOKEN_RIGHT_BRACE);
	case ';': return make_token(scanner, TOKEN_SEMICOLON);
	case ',': return make_token(scanner, TOKEN_COMMA);
    case '.': return make_token(scanner, TOKEN_DOT);
	case '-': return make_token(scanner, TOKEN_MINUS);
	case '+': return make_token(scanner, TOKEN_PLUS);
	case '/': return make_token(scanner, TOKEN_SLASH);
	case '%': return make_token(scanner, TOKEN_PERCENT);
	case '*': return make_token(scanner, TOKEN_STAR);
	case '!':
		return make_token(scanner, match(scanner, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
	case '=':
		return make_token(scanner, match(scanner, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
	case '<':
		return make_token(scanner, match(scanner, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
	case '>':
		return make_token(scanner, match(scanner, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
	case '"': return scan_string(scanner);
	}
	return error_token(scanner, "Unexpected character.");
}
