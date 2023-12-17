#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>




/*
Converts image grey.
*/
void grey(const uint8_t* img_in, uint8_t* img_out, size_t width, size_t height, float a, float b, float c);

/*
Applies the Laplace Filter to a greyed image.
*/
void laplaceFilter(const uint8_t* img_in, uint8_t* img_out, size_t width, size_t height);

/*
Blurs the image.
*/
void blur(const uint8_t* img_in, uint8_t* img_out, size_t width, size_t height);

/*
The main function using the 3 methods to denoise the given image.
*/
void denoise(const uint8_t* img, size_t width, size_t height,float a, float b, float c,uint8_t* tmp1, uint8_t* tmp2,uint8_t* result);

/*
-------------------------------------------------------------Helper Functions------------------------------------------------------------------------
*/


/*
Converts a pixel grey, using the ITU-R BT.601 standards.
*/
uint8_t greyPixel(uint8_t red, uint8_t green, uint8_t blue, float a, float b, float c);

/*
Calculates size of header
*/
int headerSize(const uint8_t* file);