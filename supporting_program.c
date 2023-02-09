#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "string.h"
#include "unistd.h"
#include "bitmap.h"
#include <getopt.h>

 char* inputPath;
 char* outputPath = "sample.bmp";
 uint8_t version = 0;
 uint8_t benchmark = 0;
 uint8_t test = 0;
 
 uint8_t countOptionals = 0;
 uint8_t countVersion = 0;
 uint8_t countBenchmark = 0;
 uint8_t countOutput = 0;
 uint8_t countTest = 0;

 int iterations = 0;

static struct option long_options[] = 
{
    {"version",      required_argument,       0,  'V' },
    {"benchmark", optional_argument,       0,  'B' },
    {"outputfile",    required_argument, 0,  'o' },
    {"help",   no_argument, 0,  'h' },
    {"test",   no_argument, 0,  't' },
    {NULL,           0,                 NULL,  0   }
};

/*
* <h1>Main/Supporting program: Image decompression</h1>
* Takes command line arguments and uses them.
* <p>
* <h2>Options, which can be set <b>(possible)</b></h2>
* -V<int> version of the Implementation (-V0, -V1) (optional -> default: V0)
* -B<int> benchmarking (int: amount of iterations(optional), default value: 3) (optional -> default: no benchmarking)
* <file> inputfile
* -o<file> outputfile (optional -> default: 'sample.bmp')
* -h || --help description of the executable (optional)
* -t || --test automized tests: check for correctness and performance (optional, but must be used alone)
* @param  argc amount of command line arguments
* @param  argv pointer on argv
* @return      exitcode
* @author      team114
*/
int main(int argc, char** argv) 
{
    int currentOptional;
    char* end;
    int result_convertion = 0;

    //loop for processing optionals
    while ((currentOptional = getopt_long(argc, argv, ":V:B::o:ht", long_options, NULL)) != -1) 
    {
        countOptionals++;
        switch (currentOptional) 
        {
            case 'V':
                    countVersion++;
                    if (countVersion >= 2) 
                    {
                        fprintf(stderr, COLOR_RED "Error with amount of optionals: " COLOR_RESET "You are not allowed to use '%c' more than once\n", currentOptional);
                        return 1;
                    }
                    
                    if (test) 
                    { 
                        fprintf(stderr, COLOR_RED "Error with testing: " COLOR_RESET "You can't test and use optionals at the same time\n");
                        return 1;
                    }

                    result_convertion = strtol(optarg, &end, 10);
                    if (*end) 
                    {
                        fprintf(stderr, COLOR_RED "Conversion error: " COLOR_RESET "%s\n", end);
                        return 1;
                    }

                    if (result_convertion >= 0 && result_convertion <= 1) 
                    {
                        version = result_convertion;
                    } else 
                    {
                        fprintf(stderr, COLOR_RED "Error with version: " COLOR_RESET "Your Version %d doesn't exist (You can use V0 or V1)\nWe will use V0\n", result_convertion);
                    }

                break;
            case 'B':
                countBenchmark++;
                if (countBenchmark >= 2) 
                {
                    fprintf(stderr, COLOR_RED "Error with amount of optionals: " COLOR_RESET "You are not allowed to use '%c' more than once\n", currentOptional);
                    return 1;
                }

                if (test) 
                { 
                    fprintf(stderr, COLOR_RED "Error with testing: " COLOR_RESET "You can't test and use optionals at the same time\n");
                    return 1;
                }

                benchmark = 1;
                iterations = 3; //default value

                if(optarg != NULL) 
                {
                    result_convertion = strtol(optarg, &end, 10);
                    if (*end) 
                    {
                        fprintf(stderr, COLOR_RED "Conversion error: " COLOR_RESET "%s\n", end);
                        return 1;
                    }

                    if (result_convertion > 0) 
                    {
                        iterations = result_convertion;
                    } else 
                    {
                        fprintf(stderr, COLOR_RED "Error with amount of iterations: " COLOR_RESET "You must use a positive number of iterations (greater than zero)\n");
                        return 1;
                    }
                }

                break;
            case 't':
                countTest++;
                if (countTest >= 2) 
                {
                    fprintf(stderr, COLOR_RED "Error with amount of optionals: " COLOR_RESET "You are not allowed to use '%c' more than once\n", currentOptional);
                    return 1;
                }
                
                if(countOptionals > 1) 
                {
                    fprintf(stderr, COLOR_RED "Error with testing: " COLOR_RESET "You can't test and use optionals at the same time\n");
                    return 1; 
                }

                test = 1;

                break;
            case 'o':
                countOutput++;
                if (countOutput >= 2) 
                {
                    fprintf(stderr, COLOR_RED "Error with amount of optionals: " COLOR_RESET "You are not allowed to use '%c' more than once\n", currentOptional);
                    return 1;
                }

                if (test) 
                { 
                    fprintf(stderr, COLOR_RED "Error with testing: " COLOR_RESET "You can't test and use optionals at the same time\n");
                    return 1;
                }

                outputPath = optarg;
                
                break;
            case 'h':
                if (test) 
                { 
                    fprintf(stderr, COLOR_RED "Error with testing: " COLOR_RESET "You can't test and use optionals at the same time\n", currentOptional);
                    return 1;
                }

                printHelpMessage();
            
                return 0;
            case ':': //Missing argument for existing option
                fprintf(stderr, COLOR_RED "Error: " COLOR_RESET "Missing argument for '%c'\n", optopt);
                
                return 1;
            case '?': //Unknown option
                fprintf(stderr, COLOR_RED "Error: " COLOR_RESET "Unknown option '%c'\n", optopt);
                
                return 1;
            default:
                fprintf(stderr, COLOR_RED "Error: " COLOR_RESET "Unknonwn error\n");
                
                return 1;
        }
    }

    //Check if test was used alone
    if (test && countOptionals > 1) 
    { 
        fprintf(stderr, COLOR_RED "Error with amount of optionals: " COLOR_RESET "You are not allowed to use '%c' more than once\n");
        return 1;
    }

    if (optind < argc && test) 
    {
        fprintf(stderr, COLOR_RED "Error with testing: " COLOR_RESET "You can't test and use non-optional arguments at the same time\n");
        return 1;
    }

    if (!test) 
    {
        //optind: other non-optional arguments (important for inputPath)
        if (optind == argc) 
        {
            fprintf(stderr, COLOR_RED "Error: " COLOR_RESET "Input file missing\n");
            return 1;
        } else if (optind <= argc - 2) 
        {
            fprintf(stderr, COLOR_RED "Error: " COLOR_RESET "Too many non-optinal arguments\n");
            return 1;
        } else  
        {
            inputPath = argv[optind];
        }
    } else 
    {
        executeTests();
        return 1;
    }

    //Call the main functionality
    if (benchmark) 
    {
        callWithBenchmark(inputPath, outputPath, iterations, version); 
    } else 
    {
        callWithoutBenchmark(inputPath, outputPath, iterations, version);
    } 

    return EXIT_SUCCESS;
}

void printHelpMessage() 
{
    printf(COLOR_YELLOW "How to use the executable (after calling 'make' in the terminal):" COLOR_RESET "\n"
                        "You always have to provide './main' first in order to invoke the program.\n"
                        "Afterwards you may make use of the following options, which should be provided after calling the executable in the terminal:\n"
                       "\t-V<int> version of the implementation (-V0, -V1) (optional -> default: V0)\n"
                       "\t-B<int> benchmarking (int: amount of iterations(optional), default value: 3) (optional -> default: no benchmarking)\n"
                       "\t<file> inputfile (required)\n"
                       "\t-o<file> outputfile (optional -> default: 'sample.bmp')\n"
                       "\t-h || --help description of the executable (optional)\n"
                       "\t-t || --test automized tests: check for correctness and performance (optional, but must be used alone)\n"
                       "correct calls/Verwendungsbeispiele:\n"
                       "\t'./main --help'\n"
                       "\t'./main --test'\n"
                       "\t'./main inputfile.bmp'\n"
                       "\t'./main inputfile.bmp -o outputfile.bmp -B1000 -V1'\n"
                       "\t...\n");
}

void executeTests() 
{
    //TESTS:                
    printf(COLOR_YELLOW "Tests:" COLOR_RESET "\n\n"
    "Tests for correctness are in folder 'correctnessTests' and results in 'correctnessTests/results'\n");
    
    //Correctnesstests:
    ioReader("correctnessTests/lena_8bpp_rle_compressed.bmp", "correctnessTests/results/lenaTestResult.bmp", 0, 0);
    ioReader("correctnessTests/sky.bmp", "correctnessTests/results/skyTestResult.bmp", 0, 0);
    ioReader("correctnessTests/usdLena.bmp", "correctnessTests/results/usdLenaTestResult.bmp", 0, 0);
    ioReader("correctnessTests/onePixel.bmp", "correctnessTests/results/onePixelTestResult.bmp", 0, 0);
    ioReader("correctnessTests/FilmImg.txt", "correctnessTests/results/filmImgTestResult.bmp", 0, 0);
    ioReader("correctnessTests/fourPixel.bmp", "correctnessTests/results/fourPixelTestResult.bmp", 0, 0);
    ioReader("correctnessTests/img.bmp", "correctnessTests/results/ImgTestResult.bmp", 0, 0);
    printf("Edgecases:\n"   
            "text.txt:\n");                              
    ioReader("correctnessTests/text.txt", "correctnessTests/crapTestResult.bmp", 0, 0);
    printf("invalidMap.bmp:\n");                              
    ioReader("correctnessTests/invalidMap.bmp", "correctnessTests/invalidMapTestResult.bmp", 0, 0);
    
    //Performancetest:  
    printf("\nPerformance tests in 'performanceTests' folder:\n"
            "Performance tests are executing...\n"
            "Version 0:\n" 
            "\nLena:\n");                              
    ioReader("performanceTests/lena.bmp", "bin/lenaPerformanceResultV0.bmp", 10000, 0);
    printf("\nImgOneColor:\n");                              
    ioReader("performanceTests/oneColor.bmp", "bin/imgOneColorPerformanceResultV0.bmp", 10000, 0);
    printf("\nsheep:\n");                              
    ioReader("performanceTests/sheep.bmp", "bin/sheepPerformanceResultV0.bmp", 10000, 0);
    printf("\nImgManyColor:\n");                              
    ioReader("performanceTests/manyColor.bmp", "bin/imgManyColorPerformanceResultV0.bmp", 10000, 0);
    printf("\nVersion 1:\n" 
            "\nLena:\n");                               
    ioReader("performanceTests/lena.bmp", "bin/lenaPerformanceResultV1.bmp", 10000, 1);
    printf("\nImgOneColor:\n");                              
    ioReader("performanceTests/oneColor.bmp", "bin/imgOneColorPerformanceResultV1.bmp", 10000, 1);
    printf("\nsheep:\n");                              
    ioReader("performanceTests/sheep.bmp", "bin/sheepPerformanceResultV1.bmp", 10000, 1);
    printf("\nImgManyColor:\n");                              
    ioReader("performanceTests/manyColor.bmp", "bin/imgManyColorPerformanceResultV1.bmp", 10000, 1);
}

void callWithBenchmark(const char* ip, const char* op, int i, uint8_t v)
{
    printf("inputPath: %s, outputPath: %s\n" 
            "iterations: %i\n"
            "version: V%i\n", ip, op, i, v);
    ioReader(ip, op, i, v);
}

void callWithoutBenchmark(const char* ip, const char* op, int i, uint8_t v)
{
    printf("inputPath: %s, outputPath: %s\n" 
            "version: V%i\n", ip, op, v);
    ioReader(ip, op, i, v);
}