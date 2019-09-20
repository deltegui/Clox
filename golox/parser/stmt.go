package parser

import "golox/lexer"

type Stmt interface {
	Accept(StmtVisitor) interface{}
}

type StmtVisitor interface {
	VisitExpressionStmt(ExprStmt) interface{}
	VisitPrintStmt(PrintStmt) interface{}
	VisitVarStmt(VarStmt) interface{}
	VisitBlockStmt(BlockStmt) interface{}
	VisitIfStmt(IfStmt) interface{}
	VisitWhileStmt(WhileStmt) interface{}
	VisitFunStmt(FunStmt) interface{}
	VisitReturnStmt(ReturnStmt) interface{}
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

type BlockStmt struct {
	Statements []Stmt
}

func (b BlockStmt) Accept(visitor StmtVisitor) interface{} {
	return visitor.VisitBlockStmt(b)
}

type IfStmt struct {
	Expression Expr
	Then       Stmt
	Else       Stmt
}

func (i IfStmt) Accept(visitor StmtVisitor) interface{} {
	return visitor.VisitIfStmt(i)
}

type WhileStmt struct {
	Expression Expr
	Statement  Stmt
}

func (while WhileStmt) Accept(visitor StmtVisitor) interface{} {
	return visitor.VisitWhileStmt(while)
}

type FunStmt struct {
	Name          lexer.Token
	ArgumentNames []lexer.Token
	Body          []Stmt
}

func (fun FunStmt) Accept(visitor StmtVisitor) interface{} {
	return visitor.VisitFunStmt(fun)
}

type ReturnStmt struct {
	Keyword    lexer.Token
	Expression Expr
}

func (ret ReturnStmt) Accept(visitor StmtVisitor) interface{} {
	return visitor.VisitReturnStmt(ret)
}
