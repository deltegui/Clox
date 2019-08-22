package lexer

import "unicode"

type sourceIndexer struct {
	source       []rune
	position     int
	lastPosition int
	line         int64
}

func newSourceIndexer(source string) sourceIndexer {
	return sourceIndexer{
		source:       []rune(source),
		position:     0,
		lastPosition: 0,
		line:         1,
	}
}

func (index *sourceIndexer) avance() {
	if index.isAtEnd() {
		return
	}
	if index.currentIs('\n') {
		index.line++
	}
	index.position++
}

func (index *sourceIndexer) updateLast() {
	index.lastPosition = index.position
}

func (index sourceIndexer) getLatestLexeme() string {
	return string(index.source[index.lastPosition:index.position])
}

func (index sourceIndexer) currentIs(char rune) bool {
	if index.position >= len(index.source) {
		return false
	}
	return index.source[index.position] == char
}

func (index sourceIndexer) currentIsNumber() bool {
	if index.position >= len(index.source) {
		return false
	}
	return unicode.IsNumber(index.source[index.position])
}

func (index sourceIndexer) currentIsAlphaNumeric() bool {
	if index.position >= len(index.source) {
		return false
	}
	char := index.source[index.position]
	return unicode.IsLetter(char) || char == '_' || unicode.IsNumber(char)
}

func (index sourceIndexer) nextIsNumber() bool {
	if index.position >= len(index.source) {
		return false
	}
	return unicode.IsNumber(index.source[index.position+1])
}

func (index *sourceIndexer) consume() rune {
	current := index.source[index.position]
	index.position++
	return current
}

func (index sourceIndexer) getLine() int64 {
	return index.line
}

func (index *sourceIndexer) addNewLine() {
	index.line++
}

func (index sourceIndexer) isAtEnd() bool {
	return index.position >= len(index.source)
}

func (index *sourceIndexer) consumeUntil(char rune) {
	for !index.isAtEnd() && index.source[index.position] != char {
		index.avance()
	}
}
