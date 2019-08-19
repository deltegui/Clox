all: build

build: build-folder
	@gcc ./src/*.c -o ./build/loxc

debug: build-folder
	gcc -g ./src/*.c -o ./build/loxc

build-folder:
	@mkdir -p ./build

run: build
	@./build/loxc

clean:
	@rm -rf ./build
	@echo 'clean!'

check_leaks: debug
	valgrind ./build/loxc
