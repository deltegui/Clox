package interpreter

import (
	"fmt"
	"golox/lexer"
	"golox/parser"
)

type Interpreter struct{}

func (interpreter Interpreter) Interpret(expr parser.Expr) {
	shit := interpreter.evaluate(expr)
	fmt.Println(shit)
}

func (interpreter Interpreter) VisitBinary(expr parser.BinaryExpr) interface{} {
	left := interpreter.evaluate(expr.Left)
	right := interpreter.evaluate(expr.Right)
	switch expr.Operator.TokenType {
	case lexer.TokenEqualEqual:
		return interpreter.isEqual(left, right)
	case lexer.TokenBangEqual:
		return interpreter.isEqual(left, right)
	case lexer.TokenGreater:
		return (left.(float64)) > (right.(float64))
	case lexer.TokenGreaterEqual:
		return (left.(float64)) >= (right.(float64))
	case lexer.TokenLess:
		return (left.(float64)) < (right.(float64))
	case lexer.TokenLessEqual:
		return (left.(float64)) <= (right.(float64))
	case lexer.TokenMinus:
		return (left.(float64)) - (right.(float64))
	case lexer.TokenPlus:
		leftFloat, isLeftFloat := left.(float64)
		rightFloat, isRightFloat := right.(float64)
		if isLeftFloat && isRightFloat {
			return leftFloat + rightFloat
		}
		return (left.(string)) + (right.(string))
	case lexer.TokenStar:
		return (left.(float64)) * (right.(float64))
	case lexer.TokenSlash:
		return (left.(float64)) / (right.(float64))
	default:
		panic("FUCK!")
	}
}

func (interpreter Interpreter) VisitGrouping(expr parser.GroupingExpr) interface{} {
	return interpreter.evaluate(expr.Expression)
}

func (interpreter Interpreter) VisitLiteral(expr parser.LiteralExpr) interface{} {
	return expr.Value
}

func (interpreter Interpreter) VisitUnary(expr parser.UnaryExpr) interface{} {
	value := interpreter.evaluate(expr.Expression)
	switch expr.Operator.TokenType {
	case lexer.TokenMinus:
		return -(value.(float64))
	case lexer.TokenBang:
		return !interpreter.isTruthy(value)
	default:
		panic("Runtime Error: Cannot apply unary to value")
	}
}

func (interpreter Interpreter) evaluate(expr parser.Expr) interface{} {
	return expr.Accept(interpreter)
}

func (interpreter Interpreter) isTruthy(value interface{}) bool {
	if value == nil {
		return false
	}
	if b, ok := value.(bool); ok {
		return b
	}
	return true
}

func (interpreter Interpreter) isEqual(left interface{}, right interface{}) bool {
	if left == nil && right == nil {
		return true
	}
	if left == nil {
		return false
	}
	return left == right
}
