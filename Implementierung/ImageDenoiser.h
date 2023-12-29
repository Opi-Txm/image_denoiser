#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <immintrin.h>
#include <x86intrin.h>
#include <xmmintrin.h>
#include <smmintrin.h>




/*
Converts image grey.
*/
void grey(const uint8_t* img_in, uint8_t* img_out, size_t width, size_t height, float a, float b, float c);

/*
Applies the Laplace Filter to a greyed image.
*/
void laplaceFilter(const uint8_t* img_in, uint8_t* img_out, size_t width, size_t height);

/*
Applies Laplace Filter to a greyed image in SIMD
*/
void laplaceFilter_V1(const uint8_t* img_in, uint8_t* img_out, size_t width, size_t height);

/*
Blurs the image.
*/
void blur(const uint8_t* img_in, uint8_t* img_out, size_t width, size_t height);

/*
Blurs the image.
*/
void blur_V1(const uint8_t* img_in, uint8_t* img_out, size_t width, size_t height);

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
Compares the images for testing.
*/
void compare(const uint8_t* img1, const uint8_t* img2, size_t width, size_t height);

/*
Calculates the difference of a and b, where both __m128i contains 8 16bit values.
*/
__m128i difference16bitValues(__m128i a, __m128i b);