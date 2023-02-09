#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "bitmap.h"
#include <string.h>

//converts the compressed pixels in uncompressed pixels and writes these in img
uint8_t bmp_rld(const uint8_t* rle_data, size_t len, size_t width, size_t height, u_int8_t* img)
{
    uint32_t resSize;
    resSize = width * height;

    int position = 0; //index of next free byte in img
    int i = 0; //index of next byte to read in rle_data

    while (1) {
        uint8_t currentElement = rle_data[i];

        //!0 => normal rle mode
        if (currentElement != 0)
        {
            uint8_t count = currentElement;
            i++;

            if (i >= len)
            {
                fprintf(stderr, COLOR_RED "Error with compressed pixel data: " COLOR_RESET "bytes are missing or wrong headerinformation\n");
                return 1;
            }

            currentElement = rle_data[i]; //HexColor 0 - 256

            //writing the bytes into img
            for (int j = count; j > 0; j--)
            {
                if (position >= resSize)
                {
                    fprintf(stderr, COLOR_RED "Error with compressed pixel data: " COLOR_RESET "too many bytes or wrong headerinformation\n");
                    return 1;
                }
                
                img[position] = currentElement;
                position++;
            }
        } else // ==0 => special mode
        {
            i++;
            if (i >= len)
            {
                fprintf(stderr, COLOR_RED "Error with compressed pixel data: " COLOR_RESET "bytes are missing or wrong headerinformation\n");
                return 1;
            }

            currentElement = rle_data[i]; //special byte
            
            //checking special modes
            switch (currentElement) 
            {
                case 0: //end of Line
                //add padding if needed             
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
                case 1: //end Of bitmap
                //exit if bitmap ended too early
                    if (i != (len - 1) )
                    {
                        fprintf(stderr, COLOR_RED "Error with bitmap: " COLOR_RESET "invalid rle8 compression or wrong headerinformation\n");
                        return 1;
                    }
                    //add padding if needed 
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
                case 2: { //delta mode
                    if (i + 2 >= resSize)
                    {
                        fprintf(stderr, COLOR_RED "Error with compressed pixel data: " COLOR_RESET "bytes are missing or wrong headerinformation\n");
                        return 1;
                    }
                    
                    int moveRight = rle_data[i + 1];
                    int moveHorizontal = rle_data[i + 2];

                    //fill skipped bytes with zero byte
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
                    //absolute mode
                    uint8_t count = currentElement;
                
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

                    //checking if absolute pixel sequence is word aligned
                    if ((count + 2) % 2 != 0) 
                    {
                        i++;
                    }
                }
            }
        }

        i++;
    }
}