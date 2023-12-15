#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "ImageDenoiser.h"


void grey(const uint8_t* img_in, uint8_t* img_out, size_t width, size_t height) {
    uint8_t d;
    uint32_t begin = headerSize;
    uint32_t end = begin + (width * height) * 3 - 3;

    for (uint32_t i = begin; i <= end; i += 3) {
        d = greyPixel(img_in[i], img_in[i+1], img_in[i+2]);
        img_out[i + 0] = d;
        img_out[i + 1] = d;
        img_out[i + 2] = d;
    }
}


/* Faltungsmatrix M^L for dim(M) = 3, used in laplaceFilter()

0  1  0
1 -4  1
0  1  0

*/


void laplaceFilter(const uint8_t* img_in, uint8_t* img_out, uint8_t width, uint8_t height) {
    //assuming that img_in already has it's headers cut out and it's only the image.
    //assuming that img_in is already grey.

    //all indexs multiplied by 3, because one pixel has 3 bytes (red, green, blue). I just take the red byte because it grey and all 3 bytes has the same value.
    //also didn't think about how to perform a matrix multiplication because it is symmetric in both ways.

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint8_t val = 0;

            //the following checks for edges and won't add values if it is. (Refer to GRA 2.1: "ein Zugriff außerhalb des Definitionsbereichs der Funktionen implizit den Wert 0 annimmt.")
            if (y != 0) {
                val += img_in[3 * ((y - 1)*(width) + x)];    
            }
            if (y != height - 1) {
                val += img_in[3 * ((y + 1)*width + x)];
            }
            if (x != 0) {
                val += img_in[3 * (y*width + x - 1)];
            }
            if (x != width - 1) {
                val += img_in[3 * (y*width + x + 1)];
            }
            val -= 4* img_in[3 * (y*width + x)];

            img_out[3 * (y*width + x)] = val;
            img_out[3 * (y*width + x) + 1] = val;
            img_out[3 * (y*width + x) + 2] = val;
        }
    }
}



void denoise(const uint8_t* img, size_t width, size_t height,float a, float b, float c,uint8_t* tmp1, uint8_t* tmp2,uint8_t* result) {

    //make image grey: Q and store in uint8_t* img
    grey(img, img, width, height);

    //apply laplace filter to grey image: Q^L and store in uint8_t* tmp1
    laplaceFilter(img, tmp1, width, height);

    //apply blur: Q^W and store in uint8_t* tmp2


    //combine img, tmp1 and tmp2 like in the last formula of GRA 2.1 Funktionsweise and store it in uint8_t* result

}


/*
-------------------------------------------------------------Helper Functions------------------------------------------------------------------------
*/


uint8_t greyPixel(uint8_t red, uint8_t green, uint8_t blue) {
    uint8_t a=3, b=6, c=1; // Actually, it is a=2.99, b = 5.87 and c = 1.14. Might change it later. Also, we have to be able to choose the a,b,c from the terminal, so this is just a temporary solution.
    return (uint8_t) ((a*red + b*green + c*blue) / (a+b+c));
}

int headerSize(const uint8_t* file) {
    int offset = 2; //skip magic number

    while (file[offset] == ' ') {
        offset++;
    }
    offset++;
    while (file[offset] == ' ') {
        offset++;
    }
    offset++;
    while (file[offset] == ' ') {
        offset++;
    }
    //offset == index of maxColorValue
    return offset + 1; 
}