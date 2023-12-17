#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "ImageDenoiser.h"



void grey(const uint8_t* img_in, uint8_t* img_out, size_t width, size_t height, float a, float b, float c) {
    uint8_t d;
    uint32_t begin = headerSize(img_in);
    uint32_t end = begin + (width * height) * 3 - 3;

    for (uint32_t i = begin; i <= end; i += 3) {
        d = greyPixel(img_in[i], img_in[i+1], img_in[i+2], a, b, c);

        img_out[i + 0] = d;
        img_out[i + 1] = d;
        img_out[i + 2] = d;
    }
}

void laplaceFilter(const uint8_t* img_in, uint8_t* img_out, size_t width, size_t height) {
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


void blur(const uint8_t* img_in, uint8_t* img_out, size_t width, size_t height) {

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint8_t val = 0;

            //the following checks for edges and won't add values if it is. (Refer to GRA 2.1: "ein Zugriff außerhalb des Definitionsbereichs der Funktionen implizit den Wert 0 annimmt.")
            if (y != 0) {
                val += 2* img_in[3 * ((y - 1)*width + x)]; 

                if (x != 0) {
                    val += img_in[3 * ((y - 1)*width + x - 1)];
                } 
                if (x != width - 1) {
                    val += img_in[3 * ((y - 1)*width + x + 1)];
                }
            }
            if (y != height - 1) {
                val += 2* img_in[3 * ((y + 1)*width + x)];

                if (x != 0) {
                    val += img_in[3 * ((y + 1)*width + x - 1)];
                } 
                if (x != width - 1) {
                    val += img_in[3 * ((y + 1)*width + x + 1)];
                }
            }
            if (x != 0) {
                val += 2* img_in[3 * (y*width + x - 1)];
                if (y != 0) {
                    val += img_in[3 * ((y - 1)*width + x - 1)];
                } 
                if (y != height - 1) {
                    val += img_in[3 * ((y + 1)*width + x - 1)];
                }
            }
            if (x != width - 1) {
                val += 2* img_in[3 * (y*width + x + 1)];
                if (y != 0) {
                    val += img_in[3 * ((y - 1)*width + x + 1)];
                } 
                if (y != height - 1) {
                    val += img_in[3 * ((y + 1)*width + x + 1)];
                }
            }
            val += 4* img_in[3 * (y*width + x)];
            val /= 16;

            img_out[3 * (y*width + x)] = val;
            img_out[3 * (y*width + x) + 1] = val;
            img_out[3 * (y*width + x) + 2] = val;
        }
    }
} 



void denoise(const uint8_t* img, size_t width, size_t height,float a, float b, float c, uint8_t* tmp1, uint8_t* tmp2, uint8_t* result) {
    //if the header of img is not cut, it should be done here.

    //make image grey: Q and store in uint8_t* img
    grey(img, img, width, height, a, b, c);

    //apply laplace filter to grey image: Q^L and store in uint8_t* tmp1
    laplaceFilter(img, tmp1, width, height);

    //apply blur: Q^W and store in uint8_t* tmp2
    blur(tmp1, tmp2, width, height);
    
    //combine img, tmp1 and tmp2 like in the last formula of GRA 2.1 Funktionsweise and store it in uint8_t* result
    //result is a pgm file, not ppm.

    //ASK TUTOR: I understand abstract because laplace filter can make value negative. But img, tmp1, tmp2 are all unsigned. Doesn't make sense to me.
    for (int i = 0; i < width * height; i++) {
        result[i] = abs(tmp1[i*3]) * img[i*3] / 1020 + (1 - abs(tmp1[i*3]) / 1020) * tmp2[i*3];
    }
}


/*
-------------------------------------------------------------Helper Functions------------------------------------------------------------------------
*/


uint8_t greyPixel(uint8_t red, uint8_t green, uint8_t blue, float a, float b, float c) {
    return (uint8_t)((a*red + b*green + c*blue) / (a+b+c));
}



int headerSize(const uint8_t* file) {
    int offset = 2; //skip magic number

    for (int i = 0; i < 3; i++) { //skipping whitespace and the value of width, height and maxColorValue.
        while (file[offset] == ' ') { 
            offset++;
        }
        while (file[offset] != ' ') { 
            offset++;
        }   
    }   
    
    while (file[offset + 1] == ' ') { //to the last whitespace before the first pixel
        offset++;
    }

    return offset; 
}