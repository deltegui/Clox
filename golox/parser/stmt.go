package parser

import "golox/lexer"

type Stmt interface {
	Accept(StmtVisitor) interface{}
}

type StmtVisitor interface {
	VisitExpressionStmt(ExprStmt) interface{}
	VisitPrintStmt(PrintStmt) interface{}
	VisitVarStmt(VarStmt) interface{}
}

type ExprStmt struct {
	Expr Expr
}

func (e ExprStmt) Accept(visitor StmtVisitor) interface{} {
	return visitor.VisitExpressionStmt(e)
}

type PrintStmt struct {
	Value Expr
}

func (e PrintStmt) Accept(visitor StmtVisitor) interface{} {
	return visitor.VisitPrintStmt(e)
}

type VarStmt struct {
	Name        lexer.Token
	Initializer Expr
}

func (varStmt VarStmt) Accept(visitor StmtVisitor) interface{} {
	return visitor.VisitVarStmt(varStmt)
}
