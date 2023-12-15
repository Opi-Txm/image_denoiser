#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>



void denoise(const uint8_t* img, size_t width, size_t height,float a, float b, float c,uint8_t* tmp1, uint8_t* tmp2,uint8_t* result);

/*
Converts image grey
*/
void grey(const uint8_t* img_in, uint8_t* img_out, size_t width, size_t height);

/*
    Converts a pixel grey, using the ITU-R BT.601 standards.
*/
uint8_t greyPixel(uint8_t red, uint8_t green, uint8_t blue);

/*
Applies the Laplace Filter to a greyed image.
*/
void laplaceFilter(const uint8_t* img_in, uint8_t* img_out, uint8_t width, uint8_t height);


/*
Calculates size of header
*/
int headerSize(const uint8_t* file);