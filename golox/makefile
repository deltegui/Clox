all: build

build: build-folder
	go build ./main.go
	mv ./main ./build/golox

build-folder:
	mkdir -p ./build

clean:
	rm -rf ./build