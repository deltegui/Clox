package main

import (
	"bufio"
	"fmt"
	"golox/interpreter"
	"golox/lexer"
	"golox/parser"
	"io/ioutil"
	"log"
	"os"
)

func main() {
	if len(os.Args) == 2 {
		runFile(os.Args[1])
		return
	}
	if len(os.Args) == 1 {
		runPrompt()
		return
	}
	log.Fatalln("Usage: lox [FILE]")
}

func runFile(filePath string) {
	content, err := ioutil.ReadFile(filePath)
	if err != nil {
		log.Fatalf("Error while reading file: %s\n", err)
	}
	run(string(content))
}

func runPrompt() {
	in := bufio.NewReader(os.Stdin)
    interpreter := interpreter.CreateInterpreter()
	for {
		fmt.Printf("(lox) ~> ")
		line, err := in.ReadString('\n')
		if err != nil {
			log.Fatalf("Error while reading line: %s\n", err)
		}
		runWithInterpreter(line, interpreter)
	}
}

func run(source string) {
    runWithInterpreter(source, interpreter.CreateInterpreter())
}

func runWithInterpreter(source string, interpreter interpreter.Interpreter) {
	lexer := lexer.NewLexer(source)
	tokens := lexer.ScanTokens()
	/*fmt.Println("--LEXER-------------------------------------")
	for _, t := range tokens {
		fmt.Println(t.String())
	}
	fmt.Println("--PARSER------------------------------------")*/
	par := parser.NewParser(tokens)
	//printer := parser.PrettyPrinter{}
	syntaxTree, err := par.Parse()
	if err != nil {
		fmt.Println(err)
		return
	}
	//printer.Print(syntaxTree)
	//fmt.Println()
	//fmt.Println("--INTERPRETER--------------------------------")
	interpreter.Interpret(syntaxTree)
}
