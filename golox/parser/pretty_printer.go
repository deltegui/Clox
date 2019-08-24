package parser

import "fmt"

type PrettyPrinter struct{}

func (printer PrettyPrinter) Print(expr Expr) interface{} {
	expr.Accept(printer)
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
