package lexer

import "fmt"

// TokenType represent a known Lox token
type TokenType string

//Single-character tokens
const (
	TokenLeftParen  TokenType = "LeftParen"
	TokenRightParen TokenType = "RightParen"
	TokenLeftBrace  TokenType = "LeftBrace"
	TokenRightBrace TokenType = "RightBrace"
	TokenComma      TokenType = "Comma"
	TokenMinus      TokenType = "Minus"
	TokenPlus       TokenType = "Plus"
	TokenSemicolon  TokenType = "Semicolon"
	TokenSlash      TokenType = "Slash"
	TokenStar       TokenType = "Star"
	TokenDot        TokenType = "Dot"
	TokenPercent	TokenType = "Percent"
)

//One or two character tokens
const (
	TokenBang         TokenType = "Bang"
	TokenBangEqual    TokenType = "BangEqual"
	TokenEqual        TokenType = "Equal"
	TokenEqualEqual   TokenType = "EqualEqual"
	TokenGreater      TokenType = "Greater"
	TokenGreaterEqual TokenType = "GreaterEqual"
	TokenLess         TokenType = "Less"
	TokenLessEqual    TokenType = "LessEqual"
)

//Literals tokens
const (
	TokenIdentifier TokenType = "Identifier"
	TokenString     TokenType = "String"
	TokenNumber     TokenType = "Number"
)

//Keywords
const (
	TokenAnd    TokenType = "And"
	TokenOr     TokenType = "Or"
	TokenElse   TokenType = "Else"
	TokenIf     TokenType = "If"
	TokenTrue   TokenType = "True"
	TokenFalse  TokenType = "False"
	TokenFun    TokenType = "Fun"
	TokenFor    TokenType = "For"
	TokenNil    TokenType = "Nil"
	TokenPrint  TokenType = "Print"
	TokenReturn TokenType = "Return"
	TokenSuper  TokenType = "Super"
	TokenThis   TokenType = "This"
	TokenVar    TokenType = "Var"
	TokenWhile  TokenType = "While"
	TokenClass  TokenType = "Class"
)

//EOF : EndOfFile token
const EOF TokenType = "EOF"

//Token is a source code in lox token
type Token struct {
	TokenType TokenType
	Lexeme    string
	Literal   interface{}
	Line      int64
}

func (t Token) String() string {
	return fmt.Sprintf("Type: %s, Lexeme: %s, Line %d, Literal: %s", t.TokenType, t.Lexeme, t.Line, t.Literal)
}

func newToken(tokenType TokenType, lexeme string, line int64) Token {
	return newTokenWithLiteral(tokenType, lexeme, line, "")
}

func newTokenWithLiteral(tokenType TokenType, lexeme string, line int64, literal interface{}) Token {
	return Token{
		TokenType: tokenType,
		Lexeme:    lexeme,
		Literal:   literal,
		Line:      line,
	}
}
