package interpreter

import (
	"golox/lexer"
	"log"
)

type environment struct {
	values    map[string]interface{}
	defined   map[string]bool
	enclosing *environment
}

func createEnvironment() environment {
	return environment{
		values:    make(map[string]interface{}),
		defined:   make(map[string]bool),
		enclosing: nil,
	}
}

func createEnvironmentFrom(enclosing environment) environment {
	return environment{
		values:    make(map[string]interface{}),
		defined:   make(map[string]bool),
		enclosing: &enclosing,
	}
}

func (env environment) define(name string, value interface{}) {
	env.values[name] = value
	env.defined[name] = true
}

func (env environment) get(name lexer.Token) interface{} {
	if env.isDefined(name) {
		return env.values[name.Lexeme]
	}
	if env.enclosing != nil {
		return env.enclosing.get(name)
	}
	log.Panicf("Undefined variable %s", name)
	return nil
}

func (env environment) isDefined(name lexer.Token) bool {
	return env.defined[name.Lexeme]
}

func (env environment) assign(name lexer.Token, value interface{}) {
	if env.isDefined(name) {
		env.values[name.Lexeme] = value
		return
	}
	if env.enclosing != nil {
		env.enclosing.assign(name, value)
		return
	}
	log.Panicf("Undefined variable %s", name)
}

func (env environment) getEnclosing() *environment {
	return env.enclosing
}
