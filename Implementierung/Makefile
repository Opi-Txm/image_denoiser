# Compiler 
CC = gcc 

# Flags 
CFLAGS=-O0 -Wall -Wextra -Wpedantic -msse4.1

LIBS=-lm

.PHONY: main clean  

all: denoise
denoise: main.c ImageDenoiser.c FileIO.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
clean:
	rm -f main