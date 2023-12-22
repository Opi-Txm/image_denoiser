#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>


struct PPMFile {
    int width;
    int height;
    int maxColorValue;
    uint8_t* data;
};

uint8_t* readPPMFile(const char* path);

bool writePGMFile(const char* path, char* data, size_t width, size_t height, size_t maxColorValue);

