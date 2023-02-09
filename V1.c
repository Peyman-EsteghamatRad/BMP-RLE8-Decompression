#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "bitmap.h"
#include <string.h>
#include <time.h>
#include <immintrin.h>


//simd implementation 
uint8_t bmp_rld_V1(const uint8_t* rle_data, size_t len, size_t width, size_t height, u_int8_t* img)
{
    uint32_t resSize;
    resSize = width*height;
    int position = 0;
    int i = 0;
    while (1) {
        uint8_t currentElement = rle_data[i];
        if (currentElement != 0)
        {
            uint8_t count = currentElement; 
            i++;
            if (i >= len)
            {
                fprintf(stderr, COLOR_RED "Error with compressed pixel data: " COLOR_RESET "bytes are missing or wrong headerinformation\n");
                return 1;
            }
            currentElement = rle_data[i];
            //filling 16 bytes in "samePixel" and writing them in memory at ones
            while (count >= 16)
            {
                if (position + 15 >= resSize)
                {
                    fprintf(stderr, COLOR_RED "Error with compressed pixel data: " COLOR_RESET "too many bytes or wrong headerinformation\n");
                    return 1;
                }
                __m128i samePixels = _mm_set1_epi8(currentElement);
                _mm_storeu_si128((img + position), samePixels);
                position = position + 16;
                count = count - 16;
            }
            //filling the rest
            for (int j = count; j > 0; j--){
                if (position >= resSize)
                {
                    fprintf(stderr, COLOR_RED "Error with compressed pixel data: " COLOR_RESET "too many bytes or wrong headerinformation\n");
                    return 1;
                }
                
                img[position] = currentElement;
                position++;
            }
        } else 
        {
            i++;
            if (i >= len)
            {
                fprintf(stderr, COLOR_RED "Error with compressed pixel data: " COLOR_RESET "bytes are missing or wrong headerinformation\n");
                return 1;
            }
            currentElement = rle_data[i];
            switch (currentElement) {
                case 0: 
                    while (position % width != 0)
                    {
                        if (position >= resSize)
                        {
                            fprintf(stderr, COLOR_RED "Error with compressed pixel data: " COLOR_RESET "too many bytes or wrong headerinformation\n");
                            return 1;
                        }

                        img[position] = currentElement;
                        position++;
                    }
                    break;
                case 1: 
                    if (i != (len - 1) )
                    {
                        fprintf(stderr, COLOR_RED "Error with bitmap: " COLOR_RESET "invalid rle8 compression or wrong headerinformation\n");
                        return 1;
                    }
                    while (position != resSize)
                    {
                        if (position >= resSize)
                        {
                            fprintf(stderr, COLOR_RED "Error with compressed pixel data: " COLOR_RESET "too many bytes or wrong headerinformation\n");
                            return 1;
                        }

                        img[position] = (uint8_t) 0;
                        position++;
                    }
                    return 0;
                case 2: {
                    if (i + 2 >= resSize)
                    {
                        fprintf(stderr, COLOR_RED "Error with compressed pixel data: " COLOR_RESET "bytes are missing or wrong headerinformation\n");
                        return 1;
                    } 
                    int moveRight = rle_data[i + 1];
                    int moveHorizontal = rle_data[i + 2];
                    for (int j = 0; j < moveRight + width * moveHorizontal; j++)
                    {
                        if (position >= resSize)
                        {
                            fprintf(stderr, COLOR_RED "Error with compressed pixel data: " COLOR_RESET "too many bytes or wrong headerinformation\n");
                            return 1;
                        }

                        img[position] = (u_char) 0;
                        position++;
                    } 
                    i = i + 2;
                    break;
                }
                default: {
                    uint8_t count = currentElement;
                    //filling the 16 absolut bytes in "samepixel" and writung them to img
                    while (count >= 16)
                    {
                        if (position + 15 >= resSize)
                        {
                            fprintf(stderr, COLOR_RED "Error with compressed pixel data: " COLOR_RESET "too many bytes or wrong headerinformation\n");
                            return 1;
                        }
                        if (i+16 >= resSize)
                        {
                            fprintf(stderr, COLOR_RED "Error with compressed pixel data: " COLOR_RESET "bytes are missing or wrong headerinformation\n");
                            return 1;
                        }
                        __m128i samePixels = _mm_setr_epi8(
                            *(rle_data + i + 1),
                            *(rle_data + i + 2),
                            *(rle_data + i + 3),
                            *(rle_data + i + 4),
                            *(rle_data + i + 5),
                            *(rle_data + i + 6),
                            *(rle_data + i + 7),
                            *(rle_data + i + 8),
                            *(rle_data + i + 9),
                            *(rle_data + i + 10),
                            *(rle_data + i + 11),
                            *(rle_data + i + 12),
                            *(rle_data + i + 13),
                            *(rle_data + i + 14),
                            *(rle_data + i + 15),
                            *(rle_data + i + 16)
                        );
                        _mm_storeu_si128((img+position), samePixels);
                        position = position + 16;
                        count = count - 16;
                        i = i + 16;
                    }
                    //filling the rest
                    for (int j = count; j > 0; j--) {
                        i++;
                        if (i >= resSize)
                        {
                            fprintf(stderr, COLOR_RED "Error with compressed pixel data: " COLOR_RESET "bytes are missing or wrong headerinformation\n");
                            return 1;
                        }
                        currentElement = rle_data[i];
                        if (position >= resSize)
                        {
                            fprintf(stderr, COLOR_RED "Error with compressed pixel data: " COLOR_RESET "too many bytes or wrong headerinformation\n");
                            return 1;
                        }
                        img[position] = currentElement;
                        position++;
                    }
                    if ((count + 2) % 2 != 0) {
                        i++;
                    }
                }
            }
        }

        i++;
    }
}