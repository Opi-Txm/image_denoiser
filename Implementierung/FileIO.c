#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>
#include <ctype.h>
#include "FileIO.h"

// Reads the content of a PPM File and returns it as a pointer to the raw data  
struct PPMFile readPPMFile(const char* filepath) {
    FILE* ppmFilePtr = fopen(filepath, "rb");
    struct PPMFile ppmFile;

    // Check if the ppm file doesn't exist
    if (ppmFilePtr == NULL) {
        fprintf(stderr, "ERROR: The following file does not exist!: %s\n", filepath);
        exit(1);
    }

    // Check if the ppm file is readable
    if (access(filepath, R_OK)) {
        fclose(ppmFilePtr);
        fprintf(stderr, "ERROR: Not allowed to read this file!\n");
        exit(1);
    }

    // Check for the magic number 
    char magic_number[2];
    magic_number[0] = fgetc(ppmFilePtr);
    magic_number[1] = fgetc(ppmFilePtr);

    if (magic_number[0] != 'P' || magic_number[1] != '6') {
        fprintf(stderr, "Error: Wrong data format!\n");
        exit(1);
    }

    // Skip the whitespaces after the magic number
    skipWhitespaces(ppmFilePtr);

    // Skip possible comment after the magic number 
    skipComments(ppmFilePtr);

    // Skip the whitespaces after the comment
    skipWhitespaces(ppmFilePtr);

    // Read the width
    if(fscanf(ppmFilePtr, "%d", &ppmFile.width) != 1) {
        printf("Error: Could not read the header successfully.");
        exit(1);
    }

    // Skip the whitespaces after the width
    skipWhitespaces(ppmFilePtr);

    // Skip possible comment after the width 
    skipComments(ppmFilePtr);

    // Read the height
    if(fscanf(ppmFilePtr, "%d", &ppmFile.height) != 1) {
        printf("Error: Could not read the header successfully.");
        exit(1);
    }

    // Skip the whitespace after the height
    skipWhitespaces(ppmFilePtr);

    // Skip possible comment after the height 
    skipComments(ppmFilePtr);

    // Read maxColorValue
    if(fscanf(ppmFilePtr, "%d", &ppmFile.maxColorValue) != 1) {
        printf("Error: Could not read the header successfully.");
        exit(1);
    }

    // Skip the whitespaces after the maxColorValue
    skipWhitespaces(ppmFilePtr);

    // We calculate the number of pixels (width * height) then multiply it by 3 because each has 3 entries of the same value (R, B, G) (each 1 byte)
    size_t rawDataSize = ppmFile.width * ppmFile.height * 3;
    ppmFile.data = (uint8_t*) malloc(rawDataSize);
    size_t sz = fread(ppmFile.data, 1, rawDataSize, ppmFilePtr);
    if (sz != rawDataSize) {
        printf("Error: Could not read the image data successfully.\n");
        exit(1);
    }
    fclose(ppmFilePtr);
    return ppmFile;
}

void writePGMFile(const char* filepath, uint8_t* data, size_t width, size_t height, size_t maxColorValue) {
    FILE* pgmFilePtr = fopen(filepath, "w");

    // Check if the pgm file doesn't exist
    if (pgmFilePtr == NULL) {
        fprintf(stderr, "ERROR: The following file does not exist!: %s\n", filepath);
        exit(1);
    }

    // Write the magic number and the other metadata in the header  
    fprintf(pgmFilePtr, "P5\n%ld %ld\n%ld\n", width, height, maxColorValue);

    // Write the pixel values 
    for(size_t i = 0; i < (width * height); i++) {
        fprintf(pgmFilePtr, "%c", data[i]);
    }

    fclose(pgmFilePtr);
}

// Skips all characters after a # and before a newline
void skipComments(FILE* ppmFilePtr) {
    char c;
    if((c = fgetc(ppmFilePtr)) == '#') {
        // Go through the comment until you find a newline, when a newline is found the comment ends
        while((c = fgetc(ppmFilePtr)) != '\n') {}
    } else {
        ungetc(c, ppmFilePtr);
    }
       
}

// Skips the whitespaces 
void skipWhitespaces(FILE* ppmFilePtr) {
    char c;
    while (isspace(c = fgetc(ppmFilePtr))) {} 
    ungetc(c, ppmFilePtr);
}
