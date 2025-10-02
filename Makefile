## Simple convenience Makefile for building and running ani

.PHONY: all build debug clean run run-json install

BUILD_DIR ?= build
BIN       ?= $(BUILD_DIR)/src/ani

all: build

build:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	cmake --build $(BUILD_DIR) -j
	ln -sf $(BUILD_DIR)/compile_commands.json compile_commands.json

debug:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	cmake --build $(BUILD_DIR) -j
	ln -sf $(BUILD_DIR)/compile_commands.json compile_commands.json

run: build
	$(BIN) $(ARGS)

run-json: build
	$(BIN) -j $(ARGS)

install: build
	mkdir -p bin
	cp -f $(BIN) bin/ani

clean:
	rm -rf $(BUILD_DIR) bin
