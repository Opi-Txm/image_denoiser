#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>

struct PPMFile;

uint8_t* readFile(const char* path);

bool writeFile(const char* path, char* string, size_t size);

void readHeader(const uint8_t* file, struct PPMFile* ppmFile);