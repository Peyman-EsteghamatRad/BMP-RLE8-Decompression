#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include "bitmap.h"

//opens file in read binary modus
FILE* open_file (const char* path){
        FILE* file;

        if (!(file = fopen(path, "rb")))
        {
            perror(COLOR_RED "Error opening File" COLOR_RESET);
            return NULL;
        }

        struct stat statbuf;
        if (fstat(fileno(file), &statbuf))
        {
            perror(COLOR_RED "Error retrieving file\n" COLOR_RESET);
            goto cleanup;
        }

        if (!S_ISREG(statbuf.st_mode)||statbuf.st_size <= 0)
        {
            fprintf(stderr, COLOR_RED "Error processing file: " COLOR_RESET "Not a regular file or invalid size\n");
            goto cleanup;
        }

        return file;
        
        cleanup:
        fclose(file);
        exit(EXIT_FAILURE); 
}

//reads header of file in bitmap struct 
HEADER* read_header (FILE* file){
        HEADER *header;
        struct stat statbuf;

        if (!(header = malloc(sizeof(HEADER))))
        {
            fprintf(stderr, COLOR_RED "Error reading file: " COLOR_RESET "Could not allocate enough memory\n"); 
            goto cleanup;
        }

        if (!fread(header, sizeof(u_char), sizeof(HEADER), file))
        {
            perror(COLOR_RED "Error reading file\n" COLOR_RESET);
            goto cleanup;
        }

        if (fstat(fileno(file), &statbuf))
        {
            perror(COLOR_RED "Error retrieving file\n" COLOR_RESET);
            goto cleanup;
        }

        //setting bmSize on statbuf size, because bmSize unreliable
        (header -> bmSize = statbuf.st_size);
        
        return header;
        
        cleanup:
        fclose(file);       
        return NULL; 
}

//read data after header that will be copied into result file
u_char* read_rest_data (FILE* file, int restDataSize){
        u_char *restData;

        if (!(restData = malloc(restDataSize)))
        {
            fprintf(stderr, COLOR_RED "Error with allocating memory:" COLOR_RESET "Could not allocate enough memory\n");
            goto cleanup;
        }

        if (!fread(restData, sizeof(u_char), restDataSize, file))
        {
            fprintf(stderr, COLOR_RED "Error with reading file: " COLOR_RESET "Can not read file\n"); 
            goto cleanup;
        }
        
        return restData;
        
        cleanup:
        fclose(file);       
        return NULL; 
}

//reading the compressed pixels out of file
u_char* read_comp_pixels (FILE* file, int pixelsSize){
        u_char *pixels;

        if (!(pixels = malloc(pixelsSize)))
        {
            fprintf(stderr, COLOR_RED "Error with allocating memory:" COLOR_RESET "Could not allocate enough memory\n");
            goto cleanup;
        }

        if (!fread(pixels, sizeof(u_char), pixelsSize, file))
        {
            fprintf(stderr, COLOR_RED "Error with reading file: " COLOR_RESET "Can not read file\n");
            goto cleanup;
        }
        
        fclose(file);
        return pixels;
        
        cleanup:
        fclose(file);       
        return NULL; 
}



//will be called from main and calls the IO operations, validates Bitmap and calls the decompression function
void ioReader(const char *inputFile, const char *outputFile, int iterations, uint8_t version)
{
    HEADER *header = NULL;
    u_char *restData = NULL;
    u_char *pixels = NULL;
    u_char *img = NULL;

    FILE *file;


    if (!(file = open_file(inputFile))) 
    {
        return;
    }

    if (!(header = read_header(file))) 
    {
        goto free;
    }             

    if (!isValidBitmap(header)){
        goto free;
    }
    
    //setting bmSizeImage correct, because it can be unreliable
    (header -> bmSizeImage) = (header -> bmSize) - (header -> imageDataOffset);

    //calculate rest data size
    int restDataSize = (header -> imageDataOffset) - (sizeof(HEADER));  

    if(!(restData = read_rest_data(file, restDataSize)))
    {
        goto free;
    }

    if(!(pixels = read_comp_pixels(file, (header -> bmSizeImage))))
    {
        goto free;
    }
    
    //calculate size of result arry including padding of width (alignment)
    uint32_t resSize;
    if ((header -> width) % 4 != 0)
    {
        (header -> width) = (header -> width) + (4 - (header -> width) % 4);
    }
    resSize = (header -> width) * (header -> height);
    
    if (!(img = (uint8_t*) malloc(resSize)))
    {
        fprintf(stderr, COLOR_RED "Error with allocating memory:" COLOR_RESET "Could not allocate enough memory\n");
        goto free;
    }

    //-B option is set -> benchmarking and calculating passed time in decompression function
    if(iterations){
    double sum = 0;

    struct timespec start;
    struct timespec end;

    for(int i = 0; i < iterations; i++)
    {
        //check which implementation to use
        if (version)
        {
            clock_gettime(CLOCK_MONOTONIC, &start); 
            if(bmp_rld_V1(pixels, (header -> bmSizeImage), (header -> width), abs(header -> height), img))
            {
                goto free;
            } 
            clock_gettime(CLOCK_MONOTONIC, &end);
            
        } else
        {
            clock_gettime(CLOCK_MONOTONIC, &start);
            if(bmp_rld(pixels, (header -> bmSizeImage), (header -> width), abs(header -> height), img))
            {
                goto free;
            } 
            clock_gettime(CLOCK_MONOTONIC, &end);

        }

        sum += end.tv_sec - start.tv_sec + 1e-9 * (end.tv_nsec - start.tv_nsec);
    }

    printf("Time needed for %i iterations (in s): %f\n", iterations, sum);
    printf("Avaragetime per iteration (in s): %f\n", sum/((double) iterations));
    
    } else {
        if (version)
        {
            if(bmp_rld_V1(pixels, (header -> bmSizeImage), (header -> width), abs(header -> height), img))
            {
                goto free;
            } 
        } else
        {
            if(bmp_rld(pixels, (header -> bmSizeImage), (header -> width), abs(header -> height), img))
            {
                goto free;
            } 
        }        
    }

    //set compression to zero --> decompressed image
    header -> bmCompression = 0;

    write_file(outputFile, header, restData, restDataSize, img, abs((header -> width) * (header -> height)));
    
    //free allocated memory (even if unsuccessfull)
    free:
    free(header);
    free(restData);         
    free(pixels);
    free(img);
}

//ckecks header information to validate bitmap
uint8_t isValidBitmap(HEADER *header) 
{
    if ((header -> indicatorB) != 'B' || (header -> indicatorM) != 'M')
    {
        fprintf(stderr, COLOR_RED "Error with Bitmapheader: " COLOR_RESET "the first two bytes are invalid\n" 
        "expected: BM\n"
        "but was: %c%c\n",(header -> indicatorB), (header -> indicatorM));
        return 0;
    }
    
    if ((header -> bmCompression) != 1 )
    {
        fprintf(stderr, COLOR_RED "Error with Bitmapheader: " COLOR_RESET "this programm only deals with rle8 compression\n"
        "expected: 1\n"
        "but was: %i\n", (header -> bmCompression));
        return 0;
    }

    if ((header -> width) <= 0)
    {
        fprintf(stderr, COLOR_RED "Error with Bitmapheader: " COLOR_RESET "width must be greater than zero\n"
        "expected: > 0\n"
        "but was: %i\n", (header -> width));
        return 0;
    }

    if ((header -> height) == 0)
    {
        fprintf(stderr, COLOR_RED "Error with Bitmapheader: " COLOR_RESET "height must be greater than zero\n"
        "expected: > 0\n"
        "but was: %i\n", (header -> width));
        return 0;
    }

    if ((header -> res1) != 0 || (header -> res2) != 0 )
    {
        fprintf(stderr, COLOR_YELLOW "Warning with Bitmapheader: " COLOR_RESET "reserved bytes are not zero\n");
    }

    if ((header -> bbpPix) != 8)
    {
        fprintf(stderr, COLOR_RED "Error with Bitmapheader: " COLOR_RESET "this programm only deals with 8bpp"
        "expected: 8\n"
        "but was: %u\n", (header -> bbpPix));
        return 0;
    }
    
    return 1;
}

//writing all the collected information back into a file with uncompressed pixel data
void write_file(const char* path, const HEADER* header,
    const u_char* restData, const int rl, u_char* pixels, const int pl) 
{
    FILE *file;

    if (!(file = fopen(path, "wb")))
    {
        fprintf(stderr, COLOR_RED "Error with opening file: " COLOR_RESET "Can not open file\n");
        return;
    }

    if (!fwrite(header, 1, sizeof(HEADER), file))
    {
        fprintf(stderr, COLOR_RED "Error with writing to file: " COLOR_RESET "Can not write in file\n");
        goto close;
    }

    if (!fwrite(restData, 1, rl, file))
    {
        fprintf(stderr, COLOR_RED "Error with writing to file: " COLOR_RESET "Can not write in file\n");    
        goto close;
    }

    if (!fwrite(pixels, 1, pl, file))
    {
        fprintf(stderr, COLOR_RED "Error with writing to file: " COLOR_RESET "Can not write in file\n");
        goto close;
    }

    close:
    fclose(file);
}