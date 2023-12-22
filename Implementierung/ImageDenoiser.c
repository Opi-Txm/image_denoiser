#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <immintrin.h>
#include <x86intrin.h>
#include <xmmintrin.h>
#include <smmintrin.h>
#include "ImageDenoiser.h"



void grey(const uint8_t* img_in, uint8_t* img_out, size_t width, size_t height, float a, float b, float c) {
    for (size_t i = 0; i < width * height; i++) {
        img_out[i] = greyPixel(img_in[3*i], img_in[3*i+1], img_in[3*i+2], a, b, c);
    }
}
void laplaceFilter(const uint8_t* img_in, uint8_t* img_out, size_t width, size_t height) {
    //assuming that img_in already has it's headers cut out and it's only the image.
    //assuming that img_in is PGM.
    int32_t val;
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            val = 0;

            //the following checks for edges and won't add values if it is. (Refer to GRA 2.1: "ein Zugriff außerhalb des Definitionsbereichs der Funktionen implizit den Wert 0 annimmt.")
            if (y != 0) {
                val += img_in[(y - 1)*(width) + x];
            }
            if (y != height - 1) {
                val += img_in[(y + 1)*width + x];
            }
            if (x != 0) {
                val += img_in[y*width + x - 1];
            }
            if (x != width - 1) {
                val += img_in[y*width + x + 1];
            }
            val -= 4* img_in[y*width + x];

            img_out[y*width + x] = (uint8_t)abs(val);
        }
    }
}




void blur(const uint8_t* img_in, uint8_t* img_out, size_t width, size_t height) {
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            uint16_t val = 0;
            uint16_t divisor = 4;

            //the following checks for edges and won't add values if it is. (Refer to GRA 2.1: "ein Zugriff außerhalb des Definitionsbereichs der Funktionen implizit den Wert 0 annimmt.")
            if (y != 0) {
                val += 2* img_in[(y - 1)*width + x]; 
                divisor += 2;
                if (x != 0) {
                    val += img_in[(y - 1)*width + x - 1];
                    divisor++;
                } 
                if (x != width - 1) {
                    val += img_in[(y - 1)*width + x + 1];   
                    divisor++;
                }
            }
            if (y != height - 1) {
                val += 2* img_in[(y + 1)*width + x];
                divisor += 2;
                if (x != 0) {
                    val += img_in[(y + 1)*width + x - 1];
                    divisor++;
                } 
                if (x != width - 1) {
                    val += img_in[(y + 1)*width + x + 1];
                    divisor++;
                }
            }
            if (x != 0) {
                val += 2* img_in[y*width + x - 1];
                divisor += 2;
            }
            if (x != width - 1) {
                val += 2* img_in[y*width + x + 1]; 
                divisor += 2; 
            }
            val += 4* img_in[y*width + x];
            val /= divisor;

            img_out[y*width + x] = (uint8_t)val;
        }
    }
} 



void denoise(const uint8_t* img, size_t width, size_t height,float a, float b, float c, uint8_t* tmp1, uint8_t* tmp2, uint8_t* result) {
    //make image grey: Q and store in uint8_t* img. img is PGM!
    uint8_t* greyImage = (uint8_t*)malloc(width*height);
    grey(img, greyImage, width, height, a, b, c);

    //apply laplace filter to grey image: Q^L and store in uint8_t* tmp1
    laplaceFilter(greyImage, tmp1, width, height);

    //apply blur: Q^W and store in uint8_t* tmp2
    blur(greyImage, tmp2, width, height);
    
    //combine img, tmp1 and tmp2 like in the last formula of GRA 2.1 Funktionsweise and store it in uint8_t* result  

    for (size_t i = 0; i < width * height; i++) {
        result[i] = tmp1[i] / 1020 * greyImage[i] + (1 - tmp1[i] / 1020) * tmp2[i];
    }
    free(greyImage);
}

/*
-------------------------------------------------------------Helper Functions------------------------------------------------------------------------
*/


uint8_t greyPixel(uint8_t red, uint8_t green, uint8_t blue, float a, float b, float c) {
    return (uint8_t)((a*red + b*green + c*blue) / (a+b+c));
}
