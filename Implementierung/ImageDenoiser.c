#include "ImageDenoiser.h"

void grey(const uint8_t* img_in, uint8_t* img_out, size_t width, size_t height, float a, float b, float c) {
    for (size_t i = 0; i < width * height; i++) {
        img_out[i] = greyPixel(img_in[3*i], img_in[3*i+1], img_in[3*i+2], a, b, c);
    }
}

void grey_V1(const uint8_t* img_in, uint8_t* img_out, size_t width, size_t height, float a, float b, float c) {
    __m128 as = _mm_set1_ps(a);
    __m128 bs = _mm_set1_ps(b);
    __m128 cs = _mm_set1_ps(c);
    __m128 red, green, blue, div = _mm_set1_ps(a+b+c);
    __m128i res;
    size_t i;
    for (i = 0; i + 12 < width * height; i += 4) {
        //store the rgb values of the next 4 pixels
        red = _mm_set_ps(img_in[3*i+9], img_in[3*i+6], img_in[3*i+3], img_in[3*i]);
        green = _mm_set_ps(img_in[3*i+10], img_in[3*i+7], img_in[3*i+4], img_in[3*i+1]);
        blue = _mm_set_ps(img_in[3*i+11], img_in[3*i+8], img_in[3*i+5], img_in[3*i+2]);

        //multiply them
        red = _mm_mul_ps(red, as);
        green = _mm_mul_ps(green, bs);
        blue = _mm_mul_ps(blue, cs);

        //sum after the multiplication, red now contains the grey value
        red = _mm_add_ps(_mm_add_ps(red, green), blue);
        red = _mm_div_ps(red, div);
        //convert floats to int32
        res = _mm_cvtps_epi32(red);
        //convert int32 to uint8_t. Now res has the 4 grey values in the first 4 bytes.
        res = _mm_packus_epi16(_mm_packs_epi32(res, _mm_setzero_si128()), _mm_setzero_si128());
        //store them in img_out.
        _mm_storeu_si128((__m128i*)(img_out + i), res);
    }
    //the rest, if the number of pixel % 4 != 0.
    for (;i < width * height; i++) {
        img_out[i] = greyPixel(img_in[3*i], img_in[3*i+1], img_in[3*i+2], a, b, c);
    }
}

void laplaceFilter(const uint8_t* img_in, uint16_t* img_out, size_t width, size_t height) {
    int32_t val;
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            val = 0;
            //Each if statement checks if the pixel is on the edge of the image to avoid processing inexistent pixels. 
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

            img_out[y*width + x] = (uint16_t)abs(val);
        }
    }
}

void laplaceFilter_V1(const uint8_t* img_in, uint16_t* img_out, size_t width, size_t height) {
    __m128i zero = _mm_setzero_si128(), mid;
    //__m128i result;
    struct HiLo sumHiLo;
    //gets space for 4 HiLo structs, top, bot, left and right pixel of it.
    //Indexes in order is: Top (0), Bot (1), Left (2), Right (3)
    struct HiLo* hilos = malloc(sizeof(struct HiLo) * 4);

    if (hilos == NULL) {
        printf("Error: No memory could be allocated in laplaceFilter_V1.");
        exit(1);
    }

    for (size_t y = 0; y < height; y++) {
        //Edge case: first pixel
        uint8_t tmpTop = 0;
        if (y+1 != height) {
            tmpTop = img_in[(y+1)*width];
        }
        if (y != 0) {
            tmpTop += img_in[(y-1)*width];
        }
        img_out[y*width] = (uint16_t) abs(img_in[y*width + 1] + tmpTop - img_in[y*width]*4);
        
        size_t x;
        for (x = 1; x + 17 < width; x+=16) {
            //Load the neighbouring pixels into the HiLo pointer.
            hilos = loadHiLoLaplace(img_in, width, height, x, y, hilos);
            //Sum up top, bot, left, right values in the pointer.
            sumHiLo = sumHiLos(hilos, 4);
            //Load the middle pixel and multiply them by 4
            mid = _mm_loadu_si128((__m128i*)(img_in + y*width + x));
            struct HiLo midHiLo = {_mm_unpackhi_epi8(mid, zero), _mm_unpacklo_epi8(mid, zero)};
            midHiLo.hi = _mm_mullo_epi16(midHiLo.hi, _mm_set1_epi16(4));
            midHiLo.lo = _mm_mullo_epi16(midHiLo.lo, _mm_set1_epi16(4));

            //Calculate the difference of the sum of the neighbouring pixels and the middle pixel.
            sumHiLo = differenceHiLo(sumHiLo, midHiLo);
            
            //Store the values in img_out.
            //result = _mm_packs_epi16(sumHiLo.hi, sumHiLo.lo);
            _mm_storeu_si128((__m128i*)(img_out + y*width+x), sumHiLo.lo);
            _mm_storeu_si128((__m128i*)(img_out + y*width+x+8), sumHiLo.hi);
        }
        //Edge case: Remaining pixels at the right less than 16 in total.
        for (; x < width; x++) {
            img_out[y*width + x] = edgeLaplaceFilter(img_in, width, height, x, y);
        }
    }
    free(hilos);
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
            //Multiply the middle pixel by four, divide it with the divisor to get the average and store it.
            val += 4* img_in[y*width + x];
            val /= divisor;
            img_out[y*width + x] = (uint8_t)val;
        }
    }
} 

void blur_V1(const uint8_t* img_in, uint8_t* img_out, size_t width, size_t height) {
    __m128i result = _mm_setzero_si128();
    __m128i* pixels = malloc(sizeof(__m128i) * 9);
    if (pixels == NULL) {
        printf("Error: No memory could be allocated for \"pixels\" in blur_V1 .\n");
        exit(1);
    }
    //Hilo struct pointer to store all the pixels. Indexes in order: Top, Top right, Top left, Mid, Right, Left, Bottom, Bottom right, Bottom left
    struct HiLo* hilos = malloc(sizeof(struct HiLo) * 9);
    //HiLo struct to store the result
    struct HiLo resultHiLo;
    if (hilos == NULL) {
        printf("Error: No memory could be allocated for \"hilos\" in blur_V1.\n");
        exit(1);
    }
    for (size_t y = 0; y < height; y++) {
        size_t x;
        //Edge cases: Top/Bottom row
        if (y == 0 || y == height - 1) {
            for (x = 0; x < width; x++) {
                img_out[y*width + x] = edgeBlur(img_in, width, height, x, y);
            }
        } else {
            // Edge case: Left column
            img_out[y*width] = (uint8_t) ((4 * img_in[y*width] + 2 * img_in[y*width + 1] + 2 * img_in[(y+1)*width] + 2 * img_in[(y-1)*width] + img_in[(y+1)*width + 1] + img_in[(y-1)*width + 1]) / 12);
            
            for (x = 1; x + 17 < width; x+=16) {
                //Loads the upper side of the matrix and multiply it by the weight. 
                pixels = loadPixels(img_in, width, x, y, pixels);

                //give row of hilos (0,1,2 for top mid bot), do the calculations and return hilos
                hilos = matrixMulBlur(hilos, pixels);

                //sum up all the values in the matrix
                resultHiLo = sumHiLos(hilos, 9);

                //divide the sum by 16
                resultHiLo = binaryShiftHiLos(resultHiLo, 4);

                //convert the 16bit values to 8bit values and store them in img_out.
                result = _mm_packus_epi16(resultHiLo.lo, resultHiLo.hi);
                _mm_storeu_si128((__m128i*)(img_out + y * width + x), result);
            }
            //Edge case: Remaining pixels at the right less than 16.
            for (;x < width; x++) {
                img_out[y*width + x] = edgeBlur(img_in, width, height, x, y);
            }
        }
    }
    free(pixels);
    free(hilos);   
}

void denoise(const uint8_t* img, size_t width, size_t height,float a, float b, float c, uint8_t* tmp1, uint8_t* tmp2, uint8_t* result) {
    //new uint16_t* due to the following reasons:
    //1. Result of the laplace filter has the max value of 1020 after making it abstract and does not fit in a uint8_t.
    //2. Dividing the value by 4 beforehand and dividing the value later in the last formula by 255 would result in different values since the rest of division by 4 is cut out. And apparently you need full mathematical precision.
    //3. Just passing a different uint8_t* for the laplace filter multiplication looks pretty complex on SIMD since you don't have a modulo function and just calculating everything with uint16_t is way easier. 
    uint16_t* tmpLaplace = malloc(sizeof(uint16_t) * width * height);
    
    //make image grey: Q and store in uint8_t* tmp1. result is PGM!
    grey(img, tmp1, width, height, a, b, c);

    //apply laplace filter to grey image: Q^L and store in uint16_t* tmpLaplace
    laplaceFilter(tmp1, tmpLaplace, width, height);

    //apply blur: Q^W and store in uint8_t* tmp2
    blur(tmp1, tmp2, width, height);
    
    //combine img, tmp1 and tmp2 like in the last formula of GRA 2.1 Funktionsweise and store it in uint8_t* result  
    float laplaceValue;
    for (size_t i = 0; i < width * height; i++) {
        laplaceValue = (float)tmpLaplace[i] / 1020;
        result[i] = (uint8_t) (laplaceValue * tmp1[i] + (1 - laplaceValue) * tmp2[i]);
    }
    free(tmpLaplace);
}

void denoise_V1(const uint8_t* img, size_t width, size_t height,float a, float b, float c, uint8_t* tmp1, uint8_t* tmp2, uint8_t* result) {
    uint16_t* tmpLaplace = malloc(sizeof(uint16_t) * width * height);

    grey_V1(img, tmp1, width, height, a, b, c);
    laplaceFilter_V1(tmp1, tmpLaplace, width, height);
    blur_V1(tmp1, tmp2, width, height);
    
    float laplaceValue;
    for (size_t i = 0; i < width * height; i++) {
        laplaceValue = (float)tmpLaplace[i] / 1020;
        result[i] = (uint8_t) (laplaceValue * tmp1[i] + (1 - laplaceValue) * tmp2[i]);
    }
    free(tmpLaplace);
}

/*
-------------------------------------------------------------Helper Functions------------------------------------------------------------------------
*/

uint8_t greyPixel(uint8_t red, uint8_t green, uint8_t blue, float a, float b, float c) {
    return (uint8_t)(lroundf((a*red + b*green + c*blue) / (a+b+c)));
}

struct HiLo sumHiLos(struct HiLo* hiLos, size_t size) {
    struct HiLo result = {_mm_setzero_si128(), _mm_setzero_si128()};
    for (size_t i = 0; i < size; i++) {
        result.hi = _mm_add_epi16(result.hi, hiLos[i].hi);
        result.lo = _mm_add_epi16(result.lo, hiLos[i].lo);
    }
    return result;
}

__m128i difference16bitValues(__m128i a, __m128i b) {
    __m128i resLo, resHi, aTmp, bTmp, zero = _mm_setzero_si128();

    //convert the uint16_t values in a and b to int32_t values to be able to handle negative values.
    aTmp = _mm_unpacklo_epi16(a, zero);
    bTmp = _mm_unpacklo_epi16(b, zero);
    resLo = _mm_abs_epi32(_mm_sub_epi32(aTmp, bTmp));

    aTmp = _mm_unpackhi_epi16(a, zero);
    bTmp = _mm_unpackhi_epi16(b, zero);
    resHi = _mm_abs_epi32(_mm_sub_epi32(aTmp, bTmp));
    //returns the abstract value of the subtraction
    return _mm_packs_epi32(resLo, resHi);
}

struct HiLo differenceHiLo(struct HiLo a, struct HiLo b) {
    __m128i hi = difference16bitValues(a.hi, b.hi);
    __m128i lo = difference16bitValues(a.lo, b.lo);
    struct HiLo result = {hi, lo};
    return result;
}


uint8_t edgeBlur(const uint8_t* img_in, size_t width, size_t height, size_t x, size_t y) {
    if (x >= width || y >= height) {
        printf("???\n");
        exit(1);
    }
    uint16_t val = 0;
    uint16_t divisor = 4;
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
    return (uint8_t)val;
}

uint16_t edgeLaplaceFilter(const uint8_t* img_in, size_t width, size_t height, size_t x, size_t y) {
    int32_t val = 0;
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
    return (uint16_t)abs(val);
}

struct HiLo* loadHiLoLaplace(const uint8_t* img_in, size_t width, size_t height, size_t x, size_t y, struct HiLo* hilos) {
    __m128i top, bot, left, right, zero = _mm_setzero_si128();
    
    for (int i = 0; i < 4; i++) {
        hilos[i].hi = zero;
        hilos[i].lo = zero;
    }
    if (y != 0) {
        top = _mm_loadu_si128((__m128i*)(img_in + (y-1)*width + x));                
        hilos[0].hi = _mm_unpackhi_epi8(top, zero);
        hilos[0].lo = _mm_unpacklo_epi8(top, zero);
    }
    if (y != height - 1) {
        bot = _mm_loadu_si128((__m128i*)(img_in + (y+1)*width + x));
        hilos[1].hi = _mm_unpackhi_epi8(bot, zero);
        hilos[1].lo = _mm_unpacklo_epi8(bot, zero);
    }

    left = _mm_loadu_si128((__m128i*)(img_in + y*width + x-1));
    hilos[2].hi = _mm_unpackhi_epi8(left, zero);
    hilos[2].lo = _mm_unpacklo_epi8(left, zero);

    right = _mm_loadu_si128((__m128i*)(img_in + y*width + x+1));
    hilos[3].hi = _mm_unpackhi_epi8(right, zero);
    hilos[3].lo = _mm_unpacklo_epi8(right, zero);

    return hilos;
}

struct HiLo* matrixMulBlur(struct HiLo* hilos, __m128i* pixels) {
    __m128i zero = _mm_setzero_si128(), two = _mm_set1_epi16(2), four = _mm_set1_epi16(4);

    //Multiplies the pixels by 2 or 4, depending on the position of the pixel, according to the matrix of a gaussian blur. 
    hilos[0].hi = _mm_mullo_epi16(_mm_unpackhi_epi8(pixels[0], zero), two);
    hilos[0].lo = _mm_mullo_epi16(_mm_unpacklo_epi8(pixels[0], zero), two);
    hilos[1].hi = _mm_unpackhi_epi8(pixels[1], zero);
    hilos[1].lo = _mm_unpacklo_epi8(pixels[1], zero);
    hilos[2].hi = _mm_unpackhi_epi8(pixels[2], zero);
    hilos[2].lo = _mm_unpacklo_epi8(pixels[2], zero);
    hilos[3].hi = _mm_mullo_epi16(_mm_unpackhi_epi8(pixels[3], zero), four);
    hilos[3].lo = _mm_mullo_epi16(_mm_unpacklo_epi8(pixels[3], zero), four);    
    hilos[4].hi = _mm_mullo_epi16(_mm_unpackhi_epi8(pixels[4], zero), two);
    hilos[4].lo = _mm_mullo_epi16(_mm_unpacklo_epi8(pixels[4], zero), two);            
    hilos[5].hi = _mm_mullo_epi16(_mm_unpackhi_epi8(pixels[5], zero), two);
    hilos[5].lo = _mm_mullo_epi16(_mm_unpacklo_epi8(pixels[5], zero), two);     
    hilos[6].hi = _mm_mullo_epi16(_mm_unpackhi_epi8(pixels[6], zero), two);
    hilos[6].lo = _mm_mullo_epi16(_mm_unpacklo_epi8(pixels[6], zero), two);
    hilos[7].hi = _mm_unpackhi_epi8(pixels[7], zero);
    hilos[7].lo = _mm_unpacklo_epi8(pixels[7], zero);
    hilos[8].hi = _mm_unpackhi_epi8(pixels[8], zero);
    hilos[8].lo = _mm_unpacklo_epi8(pixels[8], zero);

    return hilos;
}

__m128i* loadPixels(const uint8_t* img_in, size_t width, size_t x, size_t y, __m128i* pixels) {
    pixels[0] = _mm_loadu_si128((__m128i*)(img_in + (y-1)*width + x));
    pixels[1] = _mm_loadu_si128((__m128i*)(img_in + (y-1)*width + x + 1));
    pixels[2] = _mm_loadu_si128((__m128i*)(img_in + (y-1)*width + x - 1));
    pixels[3] = _mm_loadu_si128((__m128i*)(img_in + y*width + x));
    pixels[4] = _mm_loadu_si128((__m128i*)(img_in + y*width + x+1));
    pixels[5] = _mm_loadu_si128((__m128i*)(img_in + y*width + x-1));
    pixels[6]= _mm_loadu_si128((__m128i*)(img_in + (y+1)*width + x));
    pixels[7]= _mm_loadu_si128((__m128i*)(img_in + (y+1)*width + x + 1));
    pixels[8]= _mm_loadu_si128((__m128i*)(img_in + (y+1)*width + x - 1));
    return pixels;
}

struct HiLo binaryShiftHiLos(struct HiLo hilo, int shiftNum) {
    hilo.hi = _mm_srli_epi16(hilo.hi, shiftNum);
    hilo.lo = _mm_srli_epi16(hilo.lo, shiftNum);
    return hilo;
}