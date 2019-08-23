package parser

import "fmt"

type PrettyPrinter struct{}

func (printer PrettyPrinter) Print(expr Expr) {
	expr.Accept(printer)
}

func (printer PrettyPrinter) VisitBinary(expr BinaryExpr) {
	fmt.Printf(" (%s, ", expr.Operator.TokenType)
	expr.Left.Accept(printer)
	fmt.Print(", ")
	expr.Right.Accept(printer)
	fmt.Print(") ")
}

func (printer PrettyPrinter) VisitGrouping(expr GroupingExpr) {
	fmt.Print(" (group ")
	expr.Expression.Accept(printer)
	fmt.Print(") ")
}

func (printer PrettyPrinter) VisitLiteral(expr LiteralExpr) {
	fmt.Printf(" %s ", expr.Value)
}

func (printer PrettyPrinter) VisitUnary(expr UnaryExpr) {
	fmt.Printf(" (%s ", expr.Operator.TokenType)
	expr.Expression.Accept(printer)
	fmt.Print(") ")
}
