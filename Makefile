CC = gcc
CFLAGS = -Wall -Wextra -g

C_SRC = $(shell find . -type f -name '*.c')
TESTS = $(shell find tests -type f -name '*.elf')

build:
	$(CC) $(CFLAGS) $(C_SRC) -o grop

build_tests:
	cd tests && make

run_tests: build build_tests
	./grop $(TESTS)
