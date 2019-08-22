package main

import (
	"bufio"
	"fmt"
	"golox/lexer"
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
	for {
		fmt.Printf("(lox) ~> ")
		line, err := in.ReadString('\n')
		if err != nil {
			log.Fatalf("Error while reading line: %s\n", err)
		}
		run(line)
	}
}

func run(source string) {
	lexer := lexer.NewLexer(source)
	tokens := lexer.ScanTokens()
	for _, t := range tokens {
		fmt.Println(t.String())
	}
}
