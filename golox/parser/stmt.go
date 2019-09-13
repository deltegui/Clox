package parser

type Stmt interface {
	Accept(StmtVisitor) interface{}
}

type StmtVisitor interface {
	VisitExpressionStmt(ExprStmt) interface{}
	VisitPrintStmt(PrintStmt) interface{}
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
