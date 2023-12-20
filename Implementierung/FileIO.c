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
    size_t width, height, maxColorValue;
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
    fscanf(ppmFilePtr, "%ld %ld %ld", &width, &height, &maxColorValue);

    // Skip the newline after the maxColorValue
    fgetc(ppmFilePtr);

    // We calculate the number of pixels (width * height) then multiply it by 3 because each has 3 entries of the same value (R, B, G) (each 1 byte)
    size_t rawDataSize = width * height * 3;
    rawData = (uint8_t*) malloc(rawDataSize);
    fread(rawData, rawDataSize, 1, ppmFilePtr);
    fclose(ppmFilePtr);
    return rawData;
}

bool writePGMFile(const char* filepath, char* data, size_t width, size_t height, size_t maxColorValue) {
    FILE* pgmFilePtr = fopen(filepath, "w");

    // Check if the pgm file doesn't exist
    if (pgmFilePtr == NULL) {
        fprintf(stderr, "ERROR: The following file does not exist!: %s\n", filepath);
        return false;
    }

    // Write the magic number and the other metadata in the header  
    fprintf(pgmFilePtr, "P2\n%ld %ld\n%ld\n", width, height, maxColorValue);

    // Write the pixel values 
    for(size_t i = 0; i < (width * height); i++) {
        fprintf(pgmFilePtr, "%c", data[i]);
    }

    fclose(pgmFilePtr);
    return true;
}


