#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>


struct PPMFile;

uint8_t* readPPMFile(const char* path);

bool writePGMFile(const char* path, char* string, size_t size);

