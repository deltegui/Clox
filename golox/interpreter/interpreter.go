package interpreter

import (
	"fmt"
	"golox/lexer"
	"golox/parser"
	"os"
)

type Interpreter struct{}

func (interpreter Interpreter) Interpret(program []parser.Stmt) {
	defer func() {
		if r := recover(); r != nil {
			fmt.Println(r)
			os.Exit(1)
		}
	}()
	for _, statement := range program {
		statement.Accept(interpreter)
	}
}

func (interpreter Interpreter) VisitBinary(expr parser.BinaryExpr) interface{} {
	left := interpreter.evaluate(expr.Left)
	right := interpreter.evaluate(expr.Right)
	leftFloat, isLeftFloat := left.(float64)
	rightFloat, isRightFloat := right.(float64)
	switch expr.Operator.TokenType {
	case lexer.TokenEqualEqual:
		return interpreter.isEqual(left, right)
	case lexer.TokenBangEqual:
		return !interpreter.isEqual(left, right)
	case lexer.TokenGreater:
		if !isLeftFloat || !isRightFloat {
			panic("Cannot compare something and number")
		}
		return (left.(float64)) > (right.(float64))
	case lexer.TokenGreaterEqual:
		if !isLeftFloat || !isRightFloat {
			panic("Cannot compare something and number")
		}
		return (left.(float64)) >= (right.(float64))
	case lexer.TokenLess:
		if !isLeftFloat || !isRightFloat {
			panic("Cannot compare something and number")
		}
		return (left.(float64)) < (right.(float64))
	case lexer.TokenLessEqual:
		if !isLeftFloat || !isRightFloat {
			panic("Cannot compare something and number")
		}
		return (left.(float64)) <= (right.(float64))
	case lexer.TokenMinus:
		if !isLeftFloat || !isRightFloat {
			panic("Cannot substract something and number")
		}
		return (left.(float64)) - (right.(float64))
	case lexer.TokenPlus:
		if isLeftFloat && isRightFloat {
			return leftFloat + rightFloat
		}
		if isLeftFloat || isRightFloat {
			panic("Cannot sum string with number")
		}
		return (left.(string)) + (right.(string))
	case lexer.TokenStar:
		if !isLeftFloat || !isRightFloat {
			panic("Cannot multiply something and number")
		}
		return (left.(float64)) * (right.(float64))
	case lexer.TokenSlash:
		if !isLeftFloat || !isRightFloat {
			panic("Cannot divide something and number")
		}
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

func (interpreter Interpreter) VisitExpressionStmt(stmt parser.ExprStmt) interface{} {
	interpreter.evaluate(stmt.Expr)
	return nil
}

func (interpreter Interpreter) VisitPrintStmt(print parser.PrintStmt) interface{} {
	result := interpreter.evaluate(print.Value)
	fmt.Println(result)
	return nil
}

func (interpreter Interpreter) VisitVar(expr parser.VarExpr) interface{} {
	return nil
}

func (interpreter Interpreter) VisitVarStmt(stmt parser.VarStmt) interface{} {
	return nil
}