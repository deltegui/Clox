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

func (s *tokenStream) checkCurrent(tokenType lexer.TokenType, panicMessage string) {
	if s.tokens[s.current].TokenType != tokenType {
		log.Panicf("%s in line %d\n", panicMessage, s.tokens[s.current].Line)
	}
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

type Parser struct {
	tokens *tokenStream
}

func NewParser(tokens []lexer.Token) Parser {
	return Parser{&tokenStream{tokens, 0}}
}

func (p Parser) Parse() (syntaxTree Expr, err error) {
	defer func() {
		if r := recover(); r != nil {
			fmt.Println(r)
			syntaxTree = nil
			err = fmt.Errorf("Parse Error")
		}
	}()
	return p.expression(), nil
}

func (p Parser) expression() Expr {
	return p.equality()
}

func (p Parser) equality() Expr {
	fmt.Printf("EQUALITY, CURRENT %s\n", p.tokens.getCurrent().String())
	expr := p.comparison()
	for p.tokens.isCurrentEqual(lexer.TokenBangEqual, lexer.TokenEqualEqual) {
		operator := p.tokens.consume()
		left := p.comparison()
		return BinaryExpr{
			Right:    expr,
			Operator: operator,
			Left:     left,
		}
	}
	fmt.Printf("NO EQUALITY, CURRENT %s\n", p.tokens.getCurrent().String())
	return expr
}

func (p Parser) comparison() Expr {
	fmt.Printf("COMPARISION, CURRENT %s\n", p.tokens.getCurrent().String())
	expr := p.addition()
	for p.tokens.isCurrentEqual(lexer.TokenLess, lexer.TokenLessEqual, lexer.TokenGreater, lexer.TokenGreaterEqual) {
		comparator := p.tokens.consume()
		left := p.addition()
		return BinaryExpr{
			Right:    expr,
			Operator: comparator,
			Left:     left,
		}
	}
	fmt.Printf("NO COMPARISON, CURRENT %s\n", p.tokens.getCurrent().String())
	return expr
}

func (p Parser) addition() Expr {
	fmt.Printf("ADDITION, CURRENT %s\n", p.tokens.getCurrent().String())
	expr := p.multiplication()
	for p.tokens.isCurrentEqual(lexer.TokenMinus, lexer.TokenPlus) {
		fmt.Printf("FOUND ADDITION, CURRENT %s\n", p.tokens.getCurrent().String())
		addition := p.tokens.consume()
		left := p.multiplication()
		return BinaryExpr{
			Right:    expr,
			Operator: addition,
			Left:     left,
		}
	}
	fmt.Printf("NO ADDITION, CURRENT %s\n", p.tokens.getCurrent().String())
	return expr
}

func (p Parser) multiplication() Expr {
	fmt.Printf("MULTIPLICATION, CURRENT %s\n", p.tokens.getCurrent().String())
	expr := p.unary()
	for p.tokens.isCurrentEqual(lexer.TokenStar, lexer.TokenSlash) {
		fmt.Printf("FOUND MULTIPLICATION, CURRENT %s\n", p.tokens.getCurrent().String())
		multiply := p.tokens.consume()
		left := p.unary()
		return BinaryExpr{
			Right:    expr,
			Operator: multiply,
			Left:     left,
		}
	}
	fmt.Printf("NO MULTIPLICATION, CURRENT %s\n", p.tokens.getCurrent().String())
	return expr
}

func (p Parser) unary() Expr {
	fmt.Printf("UNARY, CURRENT %s\n", p.tokens.getCurrent().String())
	if p.tokens.isCurrentEqual(lexer.TokenBang, lexer.TokenMinus) {
		unary := p.tokens.consume()
		primary := p.primary()
		return UnaryExpr{
			Operator:   unary,
			Expression: primary,
		}
	}
	fmt.Printf("NO UNARY, CURRENT %s\n", p.tokens.getCurrent().String())
	return p.primary()
}

func (p Parser) primary() Expr {
	if p.tokens.isCurrentEqual(lexer.TokenTrue) {
		fmt.Printf("TRUE, CURRENT %s\n", p.tokens.getCurrent().String())
		return LiteralExpr{
			Value: true,
		}
	}
	if p.tokens.isCurrentEqual(lexer.TokenFalse) {
		fmt.Printf("FALSE, CURRENT %s\n", p.tokens.getCurrent().String())
		return LiteralExpr{
			Value: false,
		}
	}
	if p.tokens.isCurrentEqual(lexer.TokenNil) {
		fmt.Printf("NIL, CURRENT %s\n", p.tokens.getCurrent().String())
		return LiteralExpr{
			Value: nil,
		}
	}
	if p.tokens.isCurrentEqual(lexer.TokenNumber, lexer.TokenString) {
		fmt.Printf("NUMBER OR STRING, CURRENT %s\n", p.tokens.getCurrent().String())
		literal := p.tokens.consume()
		fmt.Printf("ALGO RARO, CURRENT %s\n", p.tokens.getCurrent().String())
		return LiteralExpr{
			Value: literal.Literal,
		}
	}
	if p.tokens.isCurrentEqual(lexer.TokenLeftParen) {
		fmt.Printf("GROUPING, CURRENT %s\n", p.tokens.getCurrent().String())
		p.tokens.avance()
		expr := p.expression()
		p.tokens.checkCurrent(lexer.TokenRightParen, "Expected ')'")
		return GroupingExpr{
			Expression: expr,
		}
	}
	t := p.tokens.consume()
	log.Panicf("Unexpected token: %s\n", t.String())
	return nil
}
