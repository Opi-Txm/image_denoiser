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
    for (i = 0; i + 4 <= width * height; i += 4) {
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


void laplaceFilter_V1(const uint8_t* img_in, uint8_t* img_out, size_t width, size_t height) {
    __m128i zero = _mm_setzero_si128();
    __m128i top, bot, mid, left, right;
    

    for (size_t y = 0; y < height; y++) {
        //Edge case: first pixel
        img_out[0] = (uint8_t) abs(img_in[1] + img_in[width] - img_in[0]*4);
        
        size_t x;
        for (x = 1; x + 16 < width; x+=16) {
            //load 16 uint8_t values from the image.       
            //unpack the 16 uint8_t values to 2 * 8 uint16_t values.

            
            __m128i topHi = _mm_setzero_si128();
            __m128i topLo = _mm_setzero_si128();
            if (y != 0) {
                top = _mm_loadu_si128((__m128i*)(img_in + (y-1)*width + x));
                topHi = _mm_unpackhi_epi8(top, zero);
                topLo = _mm_unpacklo_epi8(top, zero);
            }
            __m128i botHi = _mm_setzero_si128();
            __m128i botLo = _mm_setzero_si128();
            if (y != height - 1) {
                bot = _mm_loadu_si128((__m128i*)(img_in + (y+1)*width + x));
                botHi = _mm_unpackhi_epi8(bot, zero);
                botLo = _mm_unpacklo_epi8(bot, zero);
            }
            
            mid = _mm_loadu_si128((__m128i*)(img_in + y*width + x));
            __m128i midHi = _mm_unpackhi_epi8(mid, zero);
            __m128i midLo = _mm_unpacklo_epi8(mid, zero);

            left = _mm_loadu_si128((__m128i*)(img_in + y*width + x-1));
            __m128i leftHi = _mm_unpackhi_epi8(left, zero);
            __m128i leftLo = _mm_unpacklo_epi8(left, zero);

            right = _mm_loadu_si128((__m128i*)(img_in + y*width + x+1));
            __m128i rightHi = _mm_unpackhi_epi8(right, zero);
            __m128i rightLo = _mm_unpacklo_epi8(right, zero);

            //sum up top, bot, left, right values
            topHi = _mm_add_epi16(_mm_add_epi16(_mm_add_epi16(topHi, botHi), leftHi), rightHi);
            topLo = _mm_add_epi16(_mm_add_epi16(_mm_add_epi16(topLo, botLo), leftLo), rightLo);

            //multiply the middle by 4.
            midHi = _mm_mullo_epi16(midHi, _mm_set1_epi16(4));
            midLo = _mm_mullo_epi16(midLo, _mm_set1_epi16(4));

            //Perform topHi - midHi, topLo - midLo.
            midHi = difference16bitValues(topHi, midHi);
            midLo = difference16bitValues(topLo, midLo);

            //convert the 16bit values to 8bit values and store them in img_out.
            midHi = _mm_packs_epi16(midLo, midHi);
            _mm_storeu_si128((__m128i*)img_out, midHi);
        }
        //Edge case: Remaining pixels at the right less than 16 in total.
        for (;x < width; x++) {
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
    grey_V1(img, result, width, height, a, b, c);

    //apply laplace filter to grey image: Q^L and store in uint8_t* tmp1
    laplaceFilter_V1(result, tmp1, width, height);

    //apply blur: Q^W and store in uint8_t* tmp2
    blur(result, tmp2, width, height);
    
    //combine img, tmp1 and tmp2 like in the last formula of GRA 2.1 Funktionsweise and store it in uint8_t* result  

    for (size_t i = 0; i < width * height; i++) {
        result[i] = tmp1[i] / 1020 * img[i] + (1 - tmp1[i] / 1020) * tmp2[i];
    }
}

void denoise_V1(const uint8_t* img, size_t width, size_t height,float a, float b, float c, uint8_t* tmp1, uint8_t* tmp2, uint8_t* result) {
    grey_V1(img, result, width, height, a, b, c);
    laplaceFilter_V1(result, tmp1, width, height);
    blur(result, tmp2, width, height);
    for (size_t i = 0; i < width * height; i++) {
        result[i] = tmp1[i] / 1020 * img[i] + (1 - tmp1[i] / 1020) * tmp2[i];
    }
}

/*
-------------------------------------------------------------Helper Functions------------------------------------------------------------------------
*/


uint8_t greyPixel(uint8_t red, uint8_t green, uint8_t blue, float a, float b, float c) {
    return (uint8_t)((a*red + b*green + c*blue) / (a+b+c));
}


void compare(const uint8_t* img1, const uint8_t* img2, size_t width, size_t height) {
    for (size_t i = 0; i < width * height; i++) {
        if (img1[i] != img2[i]) {
            printf("Not the same!");
            break;
        }
    }
    printf("They're the same picture");
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
