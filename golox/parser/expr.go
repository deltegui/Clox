package parser

import "golox/lexer"

type Expr interface {
	Accept(ExprVisitor)
}

type ExprVisitor interface {
	VisitBinary(BinaryExpr)
	VisitGrouping(GroupingExpr)
	VisitLiteral(LiteralExpr)
	VisitUnary(UnaryExpr)
}

type BinaryExpr struct {
	Left     Expr
	Operator lexer.Token
	Right    Expr
}

func (binary BinaryExpr) Accept(visitor ExprVisitor) {
	visitor.VisitBinary(binary)
}

type GroupingExpr struct {
	Expression Expr
}

func (grouping GroupingExpr) Accept(visitor ExprVisitor) {
	visitor.VisitGrouping(grouping)
}

type LiteralExpr struct {
	Value interface{}
}

func (literal LiteralExpr) Accept(visitor ExprVisitor) {
	visitor.VisitLiteral(literal)
}

type UnaryExpr struct {
	Operator   lexer.Token
	Expression Expr
}

func (unary UnaryExpr) Accept(visitor ExprVisitor) {
	visitor.VisitUnary(unary)
}
