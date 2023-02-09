#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define COLOR_RED     "\x1b[31m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_RESET "\x1b[0m"

//header struct (some unrelevent information but still nice to have) 
typedef struct __attribute__((__packed__)) {                                                                                                                                                                                                                             
    u_char indicatorB;                                                                                                                                                                                              
    u_char indicatorM;                                                                                                                                                                                               
    uint32_t bmSize;                                                                                                                                                                                                                   
    uint16_t res1;                                                                                                                                                                                                                        
    uint16_t res2;                                                                                                                                                                                                                        
    uint32_t  imageDataOffset;
    uint32_t  dibSize;                                                                                                                                                                                                                   
    int width;                                                                                                                                                                
    int height;                                                                                                                                                                     
    uint16_t planes;                                                                                                                                                                                                                         
    uint16_t bbpPix;                                                                                                                                                                                                                         
    uint32_t bmCompression;                                                                                                                                                                                                            
    uint32_t bmSizeImage;                                                                                                                                                                                                              
    int bmXPelsPerMeter;                                                                                                                                                                                                          
    int bmYPelsPerMeter;                                                                                                                                                                                                          
    uint32_t bmClrUsed;                                                                                                                                                                                                                
    uint32_t bmClrImportant;                                                                                                                                                                                                           
}  HEADER;                                                                                                                                                                                                                                


FILE* open_file (const char* path);

HEADER* read_header (FILE* file);

u_char* read_rest_data (FILE* file, int restDataSize);

u_char* read_comp_pixels (FILE* file, int pixelsSize);

void write_file(const char* path, const HEADER* header,
    const u_char* restData, const int rl, u_char* pixels, const int pl);

void ioReader(const char *inputFile, const char *outputFile, int iterations, uint8_t version);

uint8_t bmp_rld_V1(const uint8_t* rle_data, size_t len, size_t width, size_t height, u_int8_t* img);

uint8_t bmp_rld(const uint8_t* rle_data, size_t len, size_t width, size_t height, u_int8_t* img);

uint8_t isValidBitmap();

void callWithoutBenchmark(const char* op, const char* ip, int i, uint8_t v);

void callWithBenchmark(const char* op, const char* ip, int i, uint8_t v);

void executeTests();

void printHelpMessage();