all: build

build-folder:
	mkdir ./build

build: build-folder
	gcc -lm ./*.c && mv ./a.out ./build/clox

clean:
	rm -rf ./build

.PHONY: test
test:
	./test/run_test.pl
