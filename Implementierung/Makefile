# Compiler 
CC = gcc 

# Flags 
CFLAGS=-O0 -Wall -Wextra -Wpedantic -msse4.1

.PHONY: main clean  

all: denoise
denoise: main.c ImageDenoiser.c FileIO.c
	$(CC) $(CFLAGS) -o $@ $^
clean:
	rm -f main