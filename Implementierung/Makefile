# Compiler 
CC = gcc 

# Flags 
CFLAGS=-O0 -Wall -Wextra -Wpedantic 

.PHONY: main clean  

all: denoise
denoise: main.c
	$(CC) $(CFLAGS) -o $@ $^
clean:
	rm -f main