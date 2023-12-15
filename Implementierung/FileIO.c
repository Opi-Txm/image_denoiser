#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>
struct PPMFile {
    int width;          // Width of the image
    int height;         // Height of the image
    int maxColorValue;  // Maximum color value
};


uint8_t* readFile(const char* path) {
    FILE* file = fopen(path, "rb");

    if (file == NULL) {
        fprintf(stderr, "ERROR: The following path does not exist!: %s\n", path);
        return NULL;
    }
    if (access(path, R_OK)) {
        fclose(file);
        fprintf(stderr, "ERROR: No access rights!\n");
        return NULL;
    }

    struct stat s;
    stat(path, &s);
    uint8_t* rawfile = (uint8_t *)malloc(s.st_size);
    fread(rawfile, sizeof(char) , s.st_size, file);

    fclose(file);

    return rawfile;

}

bool writeFile(const char* path, char* string, size_t size) {
    FILE* file = fopen(path, "wb");
    if (file == NULL) {
        fprintf(stderr, "ERROR: The following path does not exist!: %s\n", path);
        return false;
    }
    if (access(path, W_OK))
    {
        fclose(file);
        fprintf(stderr, "ERROR: No access rights!\n");
        return NULL;
    }
    
    fwrite(string , sizeof(char), size, file);
    fclose(file);
    return true;   
}


void readHeader(const uint8_t* file, struct PPMFile* ppmFile) {
    if(file == NULL){
        fprintf(stderr, "Error: Data not found\n");
        return;
    }
    if (file[0] != 'P' || file[1] != '6') {
        fprintf(stderr, "Error: Wrong data format!\n");
        return;
    }
    int offset = 2; //skip magic number

    //The following while loops is meant to skip the whitespaces, as it can be blanks, TABs, CRs, LFs
    while (file[offset] == ' ') {
        offset++;
    }
    sscanf(file + offset, "%d", &ppmFile->width);

    while (file[offset] == ' ') {
        offset++;
    }
    sscanf(file + offset, "%d", &ppmFile->height);

    //The Data we have to work on, which is 24bpp PPM (P6), has a constant maxColorValue of 255.
    ppmFile->maxColorValue = 255;
}