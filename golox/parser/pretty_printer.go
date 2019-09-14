package parser

import "fmt"

type PrettyPrinter struct{}

func (printer PrettyPrinter) Print(program []Stmt) interface{} {
	for _, statement := range program {
		statement.Accept(printer)
	}
	return nil
}

func (printer PrettyPrinter) VisitBinary(expr BinaryExpr) interface{} {
	fmt.Printf(" (%s, ", expr.Operator.TokenType)
	expr.Left.Accept(printer)
	fmt.Print(", ")
	expr.Right.Accept(printer)
	fmt.Print(") ")
	return nil
}

func (printer PrettyPrinter) VisitGrouping(expr GroupingExpr) interface{} {
	fmt.Print(" (group ")
	expr.Expression.Accept(printer)
	fmt.Print(") ")
	return nil
}

func (printer PrettyPrinter) VisitLiteral(expr LiteralExpr) interface{} {
	fmt.Printf(" %s ", expr.Value)
	return nil
}

func (printer PrettyPrinter) VisitUnary(expr UnaryExpr) interface{} {
	fmt.Printf(" (%s ", expr.Operator.TokenType)
	expr.Expression.Accept(printer)
	fmt.Print(") ")
	return nil
}

func (printer PrettyPrinter) VisitExpressionStmt(stmt ExprStmt) interface{} {
	fmt.Printf(" (EPXR ")
	stmt.Expr.Accept(printer)
	fmt.Print(") ")
	return nil
}

func (printer PrettyPrinter) VisitPrintStmt(print PrintStmt) interface{} {
	fmt.Printf(" (PRINT ")
	print.Value.Accept(printer)
	fmt.Print(") ")
	return nil
}

func (printer PrettyPrinter) VisitVar(expr VarExpr) interface{} {
	fmt.Printf(" (VAR REFERENCE WITH NAME %s )", expr.Name.Lexeme)
	return nil
}

func (printer PrettyPrinter) VisitVarStmt(stmt VarStmt) interface{} {
	fmt.Printf(" (VAR STATEMENT WITH NAME %s VALUE ", stmt.Name.Lexeme)
	stmt.Initializer.Accept(printer)
	fmt.Print(") ")
	return nil
}

func (printer PrettyPrinter) VisitAssign(expr AssignExpr) interface{} {
	fmt.Printf(" (ASSIGMENT WITH L-VALUE %s AND R-VALUE ", expr.Name.Lexeme)
	expr.Value.Accept(printer)
	fmt.Print(") ")
	return nil
}
