package lexer

import (
	"fmt"
	"os"
	"unicode"
)

var keywordsMap = map[string]TokenType{
	"and":    TokenAnd,
	"class":  TokenClass,
	"else":   TokenElse,
	"false":  TokenFalse,
	"for":    TokenFor,
	"fun":    TokenFun,
	"if":     TokenIf,
	"nil":    TokenNil,
	"or":     TokenOr,
	"print":  TokenPrint,
	"return": TokenReturn,
	"super":  TokenSuper,
	"this":   TokenThis,
	"true":   TokenTrue,
	"var":    TokenVar,
	"while":  TokenWhile,
}

//Lexer transform raw code to Tokens
type Lexer struct {
	source    sourceIndexer
	tokens    []Token
	hadErrors bool
}

//NewLexer frm raw source code
func NewLexer(source string) Lexer {
	return Lexer{
		source:    newSourceIndexer(source),
		tokens:    make([]Token, 0, 0),
		hadErrors: false,
	}
}

//ScanTokens from source code, reurning found tokens
func (lex *Lexer) ScanTokens() []Token {
	for !lex.source.isAtEnd() {
		lex.source.updateLast()
		lex.scanToken()
	}
	if lex.hadErrors {
		fmt.Println("There were errors during lexing...")
		os.Exit(1)
	}
	return lex.tokens
}

func (lex *Lexer) scanToken() {
	char := lex.source.consume()
	switch char {
	case '(':
		lex.addToken(TokenLeftParen)
		break
	case ')':
		lex.addToken(TokenRightParen)
		break
	case '{':
		lex.addToken(TokenLeftBrace)
		break
	case '}':
		lex.addToken(TokenRightBrace)
		break
	case ',':
		lex.addToken(TokenComma)
		break
	case '.':
		lex.addToken(TokenDot)
		break
	case '-':
		lex.addToken(TokenMinus)
		break
	case '+':
		lex.addToken(TokenPlus)
		break
	case ';':
		lex.addToken(TokenSemicolon)
		break
	case '*':
		lex.addToken(TokenStar)
		break
	case '!':
		if lex.source.currentIs('=') {
			lex.source.avance()
			lex.addToken(TokenBangEqual)
			break
		}
		lex.addToken(TokenBang)
		break
	case '=':
		if lex.source.currentIs('=') {
			lex.source.avance()
			lex.addToken(TokenEqualEqual)
			break
		}
		lex.addToken(TokenEqual)
		break
	case '>':
		if lex.source.currentIs('=') {
			lex.source.avance()
			lex.addToken(TokenGreaterEqual)
			break
		}
		lex.addToken(TokenGreater)
		break
	case '<':
		if lex.source.currentIs('=') {
			lex.source.avance()
			lex.addToken(TokenLessEqual)
			break
		}
		lex.addToken(TokenLess)
		break
	case '/':
		if lex.source.currentIs('/') {
			lex.source.consumeUntil('\n')
			break
		}
		lex.addToken(TokenSlash)
		break
	case '"':
		lex.source.consumeUntil('"')
		if lex.source.isAtEnd() {
			fmt.Printf("Unterminated string in line %d\n", lex.source.getLine())
			lex.hadErrors = true
			break
		}
		lex.source.avance()
		value := lex.source.source[lex.source.lastPosition+1 : lex.source.position-1]
		lex.addTokenLiteral(TokenString, string(value))
	case ' ':
	case '\r':
	case '\t':
		break
	case '\n':
		lex.source.addNewLine()
		break
	default:
		if unicode.IsNumber(char) {
			for lex.source.currentIsNumber() {
				lex.source.avance()
			}
			if lex.source.currentIs('.') && lex.source.nextIsNumber() {
				lex.source.avance()
				for lex.source.currentIsNumber() {
					lex.source.avance()
				}
			}
			value := lex.source.source[lex.source.lastPosition:lex.source.position]
			lex.addTokenLiteral(TokenNumber, string(value))
			break
		}
		if unicode.IsLetter(char) || char == '_' {
			for lex.source.currentIsAlphaNumeric() {
				lex.source.avance()
			}
			value := string(lex.source.source[lex.source.lastPosition:lex.source.position])
			if val, ok := keywordsMap[value]; ok {
				lex.addToken(val)
			} else {
				lex.addTokenLiteral(TokenIdentifier, value)
			}
			break
		}
		fmt.Printf("Unknown token %c in line %d\n", char, lex.source.getLine())
		lex.hadErrors = true
	}
}

func (lex *Lexer) addToken(tokenType TokenType) {
	lex.tokens = append(lex.tokens, newToken(tokenType, string(lex.source.getLatestLexeme()), lex.source.getLine()))
}

func (lex *Lexer) addTokenLiteral(tokenType TokenType, literal string) {
	lex.tokens = append(lex.tokens, newTokenWithLiteral(tokenType, string(lex.source.getLatestLexeme()), lex.source.getLine(), literal))
}
