CC := g++
CFLAGS := -Wall -g -std=c++17

OUT_DIR=bin
TARGET =bin/db
SRC_DIR=src

SRCS := $(wildcard ${SRC_DIR}/*.cpp)
OBJS := $(patsubst %.cpp,%.o,$(SRCS))

all: directories $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -v $(CFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all build clean test build-cpp directories

directories:
	mkdir -p ${OUT_DIR}

build: directories
	gcc -o bin/db main.c

test: build
	rspec

clean:
	rm -rf */*.o && rm -rf bin