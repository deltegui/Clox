package parser

import (
	"fmt"
	"golox/lexer"
	"log"
)

type tokenStream struct {
	tokens  []lexer.Token
	current int
}

func (s *tokenStream) consume() lexer.Token {
	if s.isAtEnd() {
		panic("End Of file")
	}
	s.current++
	return s.tokens[s.current-1]
}

func (s *tokenStream) checkCurrent(tokenType lexer.TokenType, panicMessage string) lexer.Token {
	if s.isAtEnd() {
		log.Panicf("%s at end", panicMessage)
	}
	if s.tokens[s.current].TokenType != tokenType {
		log.Panicf("%s in line %d\n", panicMessage, s.tokens[s.current].Line)
	}
	return s.consume()
}

func (s *tokenStream) avance() {
	if s.isAtEnd() {
		return
	}
	s.current++
}

func (s tokenStream) getCurrent() lexer.Token {
	return s.tokens[s.current]
}

func (s tokenStream) isCurrentEqual(types ...lexer.TokenType) bool {
	if s.isAtEnd() {
		return false
	}
	for _, t := range types {
		if s.tokens[s.current].TokenType == t {
			return true
		}
	}
	return false
}

func (s tokenStream) isAtEnd() bool {
	return s.current >= len(s.tokens)
}

func (s tokenStream) getCurrentLine() int64 {
	return s.getCurrent().Line
}

// Parser creates a Abstract Syntax Tree from a buffer of Tokens generated by a lexer
type Parser struct {
	tokens *tokenStream
}

// NewParser creates your parser
func NewParser(tokens []lexer.Token) Parser {
	return Parser{&tokenStream{tokens, 0}}
}

// Parse you code
func (p Parser) Parse() (program []Stmt, err error) {
	defer func() {
		if r := recover(); r != nil {
			fmt.Println(r)
			program = nil
			err = fmt.Errorf("Parse Error")
		}
	}()
	var statements []Stmt
	for !p.tokens.isAtEnd() {
		statements = append(statements, p.declaration())
	}
	return statements, nil
}

func (p Parser) declaration() Stmt {
	if p.tokens.isCurrentEqual(lexer.TokenVar) {
		return p.varDeclaration()
	}
	return p.statement()
}

func (p Parser) varDeclaration() Stmt {
	p.tokens.avance()
	tokenName := p.tokens.checkCurrent(lexer.TokenIdentifier, "Expect variable name")
	var initializer Expr = LiteralExpr{
		Value: nil,
	}
	if p.tokens.isCurrentEqual(lexer.TokenEqual) {
		p.tokens.avance()
		initializer = p.expression()
	}
	p.tokens.checkCurrent(lexer.TokenSemicolon, "Expected ; after declaration")
	return VarStmt{
		Name:        tokenName,
		Initializer: initializer,
	}
}

func (p Parser) statement() Stmt {
	if p.tokens.isCurrentEqual(lexer.TokenPrint) {
		return p.printStmt()
	}
	return p.exprStmt()
}

func (p Parser) printStmt() Stmt {
	p.tokens.avance() // consume print
	expr := p.expression()
	p.tokens.checkCurrent(lexer.TokenSemicolon, "Expected ; after value")
	return PrintStmt{
		Value: expr,
	}
}

func (p Parser) exprStmt() Stmt {
	expr := p.expression()
	p.tokens.checkCurrent(lexer.TokenSemicolon, "Expected ; after expression")
	return ExprStmt{
		Expr: expr,
	}
}

func (p Parser) expression() Expr {
	return p.assigment()
}

func (p Parser) assigment() Expr {
	expr := p.equality()
	if p.tokens.getCurrent().TokenType == lexer.TokenEqual {
		p.tokens.avance()
		value := p.assigment()
		if varExpr, ok := expr.(VarExpr); ok {
			return AssignExpr{
				Name:  varExpr.Name,
				Value: value,
			}
		}
		log.Panicf("Expected assigment in line %d", p.tokens.getCurrentLine())
	}
	return expr
}

func (p Parser) equality() Expr {
	var expr Expr
	expr = p.comparison()
	for p.tokens.isCurrentEqual(lexer.TokenBangEqual, lexer.TokenEqualEqual) {
		operator := p.tokens.consume()
		right := p.comparison()
		expr = BinaryExpr{
			Right:    right,
			Operator: operator,
			Left:     expr,
		}
	}
	return expr
}

func (p Parser) comparison() Expr {
	var expr Expr
	expr = p.addition()
	for p.tokens.isCurrentEqual(lexer.TokenLess, lexer.TokenLessEqual, lexer.TokenGreater, lexer.TokenGreaterEqual) {
		comparator := p.tokens.consume()
		right := p.addition()
		expr = BinaryExpr{
			Right:    right,
			Operator: comparator,
			Left:     expr,
		}
	}
	return expr
}

func (p Parser) addition() Expr {
	var expr Expr
	expr = p.multiplication()
	for p.tokens.isCurrentEqual(lexer.TokenMinus, lexer.TokenPlus) {
		addition := p.tokens.consume()
		right := p.multiplication()
		expr = BinaryExpr{
			Right:    right,
			Operator: addition,
			Left:     expr,
		}
	}
	return expr
}

func (p Parser) multiplication() Expr {
	var expr Expr
	expr = p.unary()
	for p.tokens.isCurrentEqual(lexer.TokenStar, lexer.TokenSlash) {
		multiply := p.tokens.consume()
		right := p.unary()
		expr = BinaryExpr{
			Right:    right,
			Operator: multiply,
			Left:     expr,
		}
	}
	return expr
}

func (p Parser) unary() Expr {
	if p.tokens.isCurrentEqual(lexer.TokenBang, lexer.TokenMinus) {
		unary := p.tokens.consume()
		primary := p.primary()
		return UnaryExpr{
			Operator:   unary,
			Expression: primary,
		}
	}
	return p.primary()
}

func (p Parser) primary() Expr {
	if p.tokens.isCurrentEqual(lexer.TokenTrue) {
		p.tokens.avance()
		return LiteralExpr{
			Value: true,
		}
	}
	if p.tokens.isCurrentEqual(lexer.TokenFalse) {
		p.tokens.avance()
		return LiteralExpr{
			Value: false,
		}
	}
	if p.tokens.isCurrentEqual(lexer.TokenNil) {
		p.tokens.avance()
		return LiteralExpr{
			Value: nil,
		}
	}
	if p.tokens.isCurrentEqual(lexer.TokenNumber, lexer.TokenString) {
		literal := p.tokens.consume()
		return LiteralExpr{
			Value: literal.Literal,
		}
	}
	if p.tokens.isCurrentEqual(lexer.TokenLeftParen) {
		p.tokens.avance()
		expr := p.expression()
		p.tokens.checkCurrent(lexer.TokenRightParen, "Expected ')'")
		return GroupingExpr{
			Expression: expr,
		}
	}
	if p.tokens.isCurrentEqual(lexer.TokenIdentifier) {
		name := p.tokens.getCurrent()
		p.tokens.avance()
		return VarExpr{
			Name: name,
		}
	}
	t := p.tokens.consume()
	log.Panicf("Unexpected token: %s\n", t.String())
	return nil
}
