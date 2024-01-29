#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <immintrin.h>
#include <x86intrin.h>
#include <xmmintrin.h>
#include <smmintrin.h>
#include <math.h>

/*
Struct HiLo represents a pair of __m128i vectors used for handling high and low parts of SIMD operations.
The 'hi' member represents the higher part, and 'lo' represents the lower part of a set of packed data.
This struct is primarily used for SIMD processing where operations are performed on multiple data points simultaneously.
*/
struct HiLo {
    __m128i hi;
    __m128i lo;
};

/*
Converts an RGB image to grayscale.
*/
void grey(const uint8_t* img_in, uint8_t* img_out, size_t width, size_t height, float a, float b, float c);
/*
Converts an RGB image to grayscale using SIMD instructions.
*/
void grey_V1(const uint8_t* img_in, uint8_t* img_out, size_t width, size_t height, float a, float b, float c);

/*
Applies a Laplace filter to a grayscale image to highlight edges.
*/
void laplaceFilter(const uint8_t* img_in, uint16_t* img_out, size_t width, size_t height);
/*
Applies a Laplace filter using SIMD instructions to a grayscale image to highlight edges.
*/
void laplaceFilter_V1(const uint8_t* img_in, uint16_t* img_out, size_t width, size_t height);

/*
Applies a blur effect to an image.
*/
void blur(const uint8_t* img_in, uint8_t* img_out, size_t width, size_t height);

/*
Applies a blur effect using SIMD to an image.
*/
void blur_V1(const uint8_t* img_in, uint8_t* img_out, size_t width, size_t height);

/*
Denoises an image by converting it to grayscale, applying a Laplace filter, and then a blur effect.
*/
void denoise(const uint8_t* img, size_t width, size_t height,float a, float b, float c,uint8_t* tmp1, uint8_t* tmp2,uint8_t* result);

/*
Denoises an image using SIMD instructions by converting it to grayscale, applying a Laplace filter, and then a blur effect.
*/
void denoise_V1(const uint8_t* img, size_t width, size_t height,float a, float b, float c,uint8_t* tmp1, uint8_t* tmp2,uint8_t* result);

/*
-------------------------------------------------------------Helper Functions------------------------------------------------------------------------
*/


/*
Converts a single RGB pixel to grayscale using the ITU-R BT.601 standard.
*/
uint8_t greyPixel(uint8_t red, uint8_t green, uint8_t blue, float a, float b, float c);

/*
Sums up multiple HiLo structs (each containing two __m128i vectors) into a single HiLo struct.
*/
struct HiLo sumHiLos(struct HiLo* hiLos, size_t size);
/*
Calculates the difference between two HiLo structs, each containing two __m128i vectors.
*/
struct HiLo differenceHiLo(struct HiLo a, struct HiLo b);
/*
Calculates the difference between two __m128i vectors, each containing eight 16-bit values.
*/
__m128i difference16bitValues(__m128i a, __m128i b);
/*
Applies the blur filter to a specific pixel in an image, handling edge cases.
*/
uint8_t edgeBlur(const uint8_t* img_in, size_t width, size_t height, size_t x, size_t y);
/*
Applies the Laplace filter to a specific pixel in an image, handling edge cases.
*/
uint16_t edgeLaplaceFilter(const uint8_t* img_in, size_t width, size_t height, size_t x, size_t y);
/*
Loads pixel data into HiLo structs for Laplace filter processing using SIMD instructions.
*/
struct HiLo* loadHiLoLaplace(const uint8_t* img_in, size_t width, size_t height, size_t x, size_t y, struct HiLo* hilos);
/*
Multiplies the HiLo structs with pixel values for the blur operation using SIMD instructions.
*/
struct HiLo* matrixMulBlur(struct HiLo* hilos, __m128i* pixels);
/*
Loads pixel data from the image into an array of __m128i vectors for SIMD processing.
*/
__m128i* loadPixels(const uint8_t* img_in, size_t width, size_t x, size_t y, __m128i* pixels);
/*
Performs a binary shift on HiLo structs to adjust the values as part of the blur process.
*/
struct HiLo binaryShiftHiLos(struct HiLo hilo, int shiftNum);