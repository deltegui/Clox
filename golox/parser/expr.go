package parser

import "golox/lexer"

type Expr interface {
	Accept(ExprVisitor) interface{}
}

type ExprVisitor interface {
	VisitBinary(BinaryExpr) interface{}
	VisitGrouping(GroupingExpr) interface{}
	VisitLiteral(LiteralExpr) interface{}
	VisitUnary(UnaryExpr) interface{}
	VisitVar(VarExpr) interface{}
	VisitAssign(AssignExpr) interface{}
}

type AssignExpr struct {
	Name  lexer.Token
	Value Expr
}

func (assign AssignExpr) Accept(visitor ExprVisitor) interface{} {
	return visitor.VisitAssign(assign)
}

type BinaryExpr struct {
	Left     Expr
	Operator lexer.Token
	Right    Expr
}

func (binary BinaryExpr) Accept(visitor ExprVisitor) interface{} {
	return visitor.VisitBinary(binary)
}

type GroupingExpr struct {
	Expression Expr
}

func (grouping GroupingExpr) Accept(visitor ExprVisitor) interface{} {
	return visitor.VisitGrouping(grouping)
}

type LiteralExpr struct {
	Value interface{}
}

func (literal LiteralExpr) Accept(visitor ExprVisitor) interface{} {
	return visitor.VisitLiteral(literal)
}

type UnaryExpr struct {
	Operator   lexer.Token
	Expression Expr
}

func (unary UnaryExpr) Accept(visitor ExprVisitor) interface{} {
	return visitor.VisitUnary(unary)
}

type VarExpr struct {
	Name lexer.Token
}

func (varExpr VarExpr) Accept(visitor ExprVisitor) interface{} {
	return visitor.VisitVar(varExpr)
}
