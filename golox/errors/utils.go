package errors

import (
	"fmt"
	"log"
)

//ErrorReport represents a way to report to the user a interpreter error.
type ErrorReport interface {
	Report(err LoxError)
}

//LoxError represents a Lox Interperter Error.
type LoxError struct {
	Type    string
	Line    int64
	Message string
	Where   string
}

func NewLexerError(message string, line int64) LoxError {
	return LoxError{
		Type:    "Lexer",
		Line:    line,
		Message: message,
		Where:   "",
	}
}

func (err LoxError) Error() string {
	return fmt.Sprintf("[%d] %s: %s in %s", err.Line, err.Type, err.Message, err.Where)
}

//ConsoleErrorReport is a ErrorReport that reports to console
type ConsoleErrorReport struct{}

// Report error to user and exit
func (report ConsoleErrorReport) Report(err LoxError) {
	log.Fatal(err.Error())
}
