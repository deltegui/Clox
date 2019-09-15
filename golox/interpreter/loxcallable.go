package interpreter

import (
	"golox/parser"
	"time"
)

type LoxCallable interface {
	Call(interpreter Interpreter, args []interface{}) interface{}
	Arity() int
}

type TimeNative struct{}

func (t TimeNative) Call(interpreter Interpreter, args []interface{}) interface{} {
	return time.Now().Unix()
}

func (t TimeNative) Arity() int {
	return 0
}

type LoxFunction struct {
	declaration parser.FunStmt
}

func (fun LoxFunction) Call(interpreter Interpreter, args []interface{}) interface{} {
	funEnv := createEnvironmentFrom(interpreter.env)
	for i := 0; i < fun.Arity(); i++ {
		funEnv.define(fun.declaration.ArgumentNames[i].Lexeme, args[i])
	}
	interpreter.env = funEnv
	interpreter.Interpret(fun.declaration.Body)
	return nil
}

func (fun LoxFunction) Arity() int {
	return len(fun.declaration.ArgumentNames)
}
