#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>

/*
Struct PPMFile represents the PPM file, containing the information of the header and all of the pixels in data. 
*/
struct PPMFile {
    int width;
    int height;
    int maxColorValue;
    uint8_t* data;
};
/*
Reads the PPM file from the filepath and return it as a struct PPMFile
*/
struct PPMFile readPPMFile(const char* filepath);
/*
Creates a file to the path (if there is no file on the path) and put the header information and image data into the file, following the PGM file format (P5)
*/
void writePGMFile(const char* path, uint8_t* data, size_t width, size_t height, size_t maxColorValue);
/*
Skips the comments
*/
void skipComments(FILE* ppmFilePtr);

