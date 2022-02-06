Project to learn how to create interpreters and compilers. All credit is for [Bob Nystrom](https://github.com/munificent) and his fantastic book [Crafting Interpreters](https://www.amazon.es/Crafting-Interpreters-Robert-Nystrom/dp/0990582930/ref=sr_1_1?keywords=crafting+interpreters&qid=1644166558&sprefix=crafting+interpre%2Caps%2C99&sr=8-1). I just followed the book and implemented his interpreter to learn how to do it. The code is not a copy-paste from the book, however. I tried to implement it by myself as a way to learn and understand what I am doing. 

# CLox
CLox is the lox interpreter made with C using a stack-based virtual machine and with a one-step compiler. It is inside the "clox" folder. To compile it you only need gcc or other C compiler and Make. Just run make to build the interpreter.

# GoLox
GoLox is the lox interpreter made with Golang. It's an abstract syntax tree interpreter, so it is easy to understand but slow. You need Go 1.13 to build the interpreter. Just run make to compile it.
