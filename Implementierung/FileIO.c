#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>
#include "FileIO.h"

// Reads the content of a PPM File and returns it as a pointer to the raw data  
uint8_t* readPPMFile(const char* filepath) {
    FILE* ppmFilePtr = fopen(filepath, "rb");
    int width, height, maxColorValue;
    uint8_t* rawData;

    // Check if the ppm file doesn't exist
    if (ppmFilePtr == NULL) {
        fprintf(stderr, "ERROR: The following file does not exist!: %s\n", filepath);
        return NULL;
    }

    // Check if the ppm file is readable
    if (access(filepath, R_OK)) {
        fclose(ppmFilePtr);
        fprintf(stderr, "ERROR: Not allowed to read this file!\n");
        return NULL;
    }

    // Check for the magic number 
    char magic_number[2];
    magic_number[0] = fgetc(ppmFilePtr);
    magic_number[1] = fgetc(ppmFilePtr);

    if (magic_number[0] != 'P' || magic_number[1] != '6') {
        fprintf(stderr, "Error: Wrong data format!\n");
        return NULL;
    }

    // Get width and height and maxColorValue (whitespaces are skipped automatically from fscanf)
    fscanf(ppmFilePtr, "%d %d %d", &width, &height, &maxColorValue);

    // We calculate the number of pixels (width * height) then multiply it by 3 because each has 3 entries of the same value (R, B, G) (each 1 byte)
    size_t rawDataSize = width * height * 3;
    rawData = (uint8_t*) malloc(rawDataSize);
    fread(rawData, rawDataSize, 1, ppmFilePtr);
    fclose(ppmFilePtr);
    return rawData;
}

bool writePGMFile(const char* path, char* string, size_t size) {

}


