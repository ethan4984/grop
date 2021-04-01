CC = gcc
CFLAGS = -Wall -Wextra

C_SRC = $(shell find . -type f -name '*.c')

build:
	$(CC) $(CFLAGS) $(C_SRC) -o grop
