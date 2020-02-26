OUT_DIR=bin

.PHONY: all build clean test

directories:
	mkdir -p ${OUT_DIR}

build: directories
	g++ -o bin/db main.cpp

test: build
	rspec

clean:
	rm -rf bin