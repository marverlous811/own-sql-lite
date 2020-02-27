OUT_DIR=bin

.PHONY: all build clean test

directories:
	mkdir -p ${OUT_DIR}

build: directories
	gcc -o bin/db main.c

test: build
	rspec

clean:
	rm -rf bin