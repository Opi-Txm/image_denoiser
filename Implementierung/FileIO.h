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

struct PPMFile readPPMFile(const char* filepath);

void writePGMFile(const char* path, uint8_t* data, size_t width, size_t height, size_t maxColorValue);

