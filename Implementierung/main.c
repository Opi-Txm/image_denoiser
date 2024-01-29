#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "FileIO.h"
#include "ImageDenoiser.h"
#include <time.h>

#define OPTIONAL_ARGUMENT_IS_PRESENT \
    ((optarg == NULL && optind < argc && argv[optind][0] != '-') \
     ? (bool) (optarg = argv[optind++]) \
     : (optarg != NULL))

/**
 * This function prints out the help message for the project
 */
void printUsage();

// --------------------------------------
//                 Main
// --------------------------------------
int main(int argc, char *argv[]) {

    char* inputFilePath = "";
    char* outputFilePath = "";
     
    struct PPMFile inputPPMFileStruct = {0, 0, 0, NULL};

    int opt;
    struct option long_options[] = {{"version",     optional_argument, NULL, 'V'},
                                    {"benchmark",   optional_argument, NULL, 'B'},
                                    {"output",      required_argument, NULL, 'o'},
                                    {"coeffs", required_argument, NULL, 'c'},
                                    {"help",        no_argument,       NULL, 'h'},
                                    {"correctness", no_argument,       NULL, 'k'}};
    // Inverse counter for the necessary arguments
    int necessary = 2;

    // Version of the implementation that is performed, default is 0
    int version = 0;

    // Standard values for the coefficients
    float coeffA = 2.99;
    float coeffB = 5.87;
    float coeffC = 1.14;

    // Whether benchmarking will be performed for the implementation at hand
    bool benchmarking = false;

    // Whether the function is called multiple times for benchmarking purposes
    bool iter = false;

    // How often the function can be called for benchmarking purposes
    int iterations = 1;

    // Test the correctness of the two implementations
    bool correctness = false;

    // Loop for processing the command line input
    while (optind < argc) {
        if((opt = getopt_long(argc, argv, "V:B::o:c:::hk", long_options, NULL)) != -1) {
            switch (opt) {
                case 'V':
                    if (strcmp(optarg, "0") != 0 || strcmp(optarg, "1") != 0) {
                        version = atoi(optarg);
                    } else {
                        fprintf(stderr, "Please provide 0 or 1 as the version\n");
                        printUsage();
                        exit(1);
                    }
                    break;
                case 'B':
                    benchmarking = true;
                    if (OPTIONAL_ARGUMENT_IS_PRESENT) {
                        iter = true;
                        iterations = atoi(optarg);
                        if (iterations <= 0) {
                            fprintf(stderr, "The number of calls needs to be > 0\n");
                            printUsage();
                            exit(1);
                        }
                    }
                    break;
                case 'o':
                    outputFilePath = optarg; // Define the output name/path of the file
                    necessary--;
                    break;
                case 'c':
                    if (OPTIONAL_ARGUMENT_IS_PRESENT) {
                        int parsed = sscanf(optarg, "%f,%f,%f", &coeffA, &coeffB, &coeffC);

                        if (parsed != 3) {
                            fprintf(stderr, "Please provide 3 coefficient values.\n");
                            printUsage();
                            exit(1);
                        }
                    } else {
                        fprintf(stderr, "Please provide the values for the coefficients!\n");
                        printUsage();
                        exit(1);
                    }
                    break;
                case 'h':
                    printUsage();
                    return 0;
                case 'k':
                    correctness = true;
                    break;
                default:
                    printUsage();
                    exit(1);
            }
        } else {
            inputFilePath = argv[optind]; // Get the input file path 
                    
            if (!strcmp(inputFilePath, "")) {
                fprintf(stderr, "Invalid Path!\n");
                exit(1);
            }

            // Reading the PPM input file into a PPMFile structure 
            inputPPMFileStruct = readPPMFile(inputFilePath);

            necessary--;
            optind++;
        }
    }

    // Print help when no arguments are present or the necessary arguments are not present
    if (necessary != 0 || argc == 1) {
        fprintf(stderr, "Not enough arguments\n");
        printUsage();
        exit(1);
    }

    // TODO: CHECK FOR a,b,c not being 0
    if (coeffA <= 0.0 || coeffB <= 0.0 || coeffC <= 0.0) {
        fprintf(stderr, "Invalid coefficient values!");
        printUsage();
        exit(1);
    }

    // Temporary variables 
    uint8_t* tempVar1;
    uint8_t* tempVar2;
    tempVar1 = (uint8_t*) malloc (inputPPMFileStruct.width * inputPPMFileStruct.height);
    if (tempVar1 == NULL) {
        fprintf(stderr, "Error: No memory could be allocated for \"tempVar1\" in main.");
        exit(1);
    }
    tempVar2 = (uint8_t*) malloc (inputPPMFileStruct.width * inputPPMFileStruct.height);
    if (tempVar2 == NULL) {
        fprintf(stderr, "Error: No memory could be allocated for \"tempVar2\" in main.");
        exit(1);
    }
    // The final output raw data 
    uint8_t* denoisedRawData = (uint8_t*) malloc (inputPPMFileStruct.width * inputPPMFileStruct.height);
    if (denoisedRawData == NULL) {
        fprintf(stderr, "Error: No memory could be allocated for \"denoisedRawData\" in main.");
        exit(1);
    }

    if (version == 0) {
        if (benchmarking) {
            if (!iter) {
                struct timespec start;
                clock_gettime(CLOCK_MONOTONIC, &start);
                // Main denoise function version 0
                denoise(inputPPMFileStruct.data, inputPPMFileStruct.width, inputPPMFileStruct.height, coeffA, coeffB, coeffC, tempVar1, tempVar2, denoisedRawData); 
                struct timespec end;
                clock_gettime(CLOCK_MONOTONIC, &end);
                double time = end.tv_sec - start.tv_sec + 1e-9 * (end.tv_nsec - start.tv_nsec);
                printf("Done after %f seconds.\n", time);
            } else {
                struct timespec start;
                clock_gettime(CLOCK_MONOTONIC, &start);
                for (int i = 1; i <= iterations; i++) {
                    denoise(inputPPMFileStruct.data, inputPPMFileStruct.width, inputPPMFileStruct.height, coeffA, coeffB, coeffC, tempVar1, tempVar2, denoisedRawData); 
                }
                struct timespec end;
                clock_gettime(CLOCK_MONOTONIC, &end);
                double time = end.tv_sec - start.tv_sec + 1e-9 * (end.tv_nsec - start.tv_nsec);
                printf("Done after %f seconds (measured on %d iterations)\n", time, iterations);
            }
        } else {
            denoise(inputPPMFileStruct.data, inputPPMFileStruct.width, inputPPMFileStruct.height, coeffA, coeffB, coeffC, tempVar1, tempVar2, denoisedRawData); 
        }
    } else if (version == 1) {
        if (benchmarking) {
            if (!iter) {
                struct timespec start;
                clock_gettime(CLOCK_MONOTONIC, &start);
                // Denoise function version 1
                denoise_V1(inputPPMFileStruct.data, inputPPMFileStruct.width, inputPPMFileStruct.height, coeffA, coeffB, coeffC, tempVar1, tempVar2, denoisedRawData); 
                struct timespec end;
                clock_gettime(CLOCK_MONOTONIC, &end);
                double time = end.tv_sec - start.tv_sec + 1e-9 * (end.tv_nsec - start.tv_nsec);
                printf("Done after %f seconds.\n", time);
            } else {
                struct timespec start;
                clock_gettime(CLOCK_MONOTONIC, &start);
                for (int i = 1; i <= iterations; i++) {
                    denoise_V1(inputPPMFileStruct.data, inputPPMFileStruct.width, inputPPMFileStruct.height, coeffA, coeffB, coeffC, tempVar1, tempVar2, denoisedRawData); 
                }
                struct timespec end;
                clock_gettime(CLOCK_MONOTONIC, &end);
                double time = end.tv_sec - start.tv_sec + 1e-9 * (end.tv_nsec - start.tv_nsec);
                printf("Done after %f seconds (measured on %d iterations)\n", time, iterations);
            }
        } else {
            denoise_V1(inputPPMFileStruct.data, inputPPMFileStruct.width, inputPPMFileStruct.height, coeffA, coeffB, coeffC, tempVar1, tempVar2, denoisedRawData);
        }

    } else {
        fprintf(stderr, "Please provide 0 or 1 as the implementation version.\n");
        printUsage();
        exit(1);
    }

    // write the denoised data back into a pgm file 
    writePGMFile(outputFilePath, denoisedRawData, inputPPMFileStruct.width, inputPPMFileStruct.height, inputPPMFileStruct.maxColorValue);


    // Check for correctness
    if (correctness) {
        uint8_t* a = (uint8_t*) malloc (inputPPMFileStruct.width * inputPPMFileStruct.height);
        if (a == NULL) {
            fprintf(stderr, "Error: No memory could be allocated for \"a\" in main/correctness.");
            free(tempVar1);
            free(tempVar2);
            free(denoisedRawData);
            free(inputPPMFileStruct.data);
            exit(1);
        }
        denoise(inputPPMFileStruct.data, inputPPMFileStruct.width, inputPPMFileStruct.height, coeffA, coeffB, coeffC, tempVar1, tempVar2, a);

        uint8_t* b = (uint8_t*) malloc (inputPPMFileStruct.width * inputPPMFileStruct.height);
        if (b == NULL) {
            fprintf(stderr, "Error: No memory could be allocated for \"b\" in main/correctness.");
            free(tempVar1);
            free(tempVar2);
            free(denoisedRawData);
            free(inputPPMFileStruct.data);
            free(a);
            exit(1);
        }
        denoise_V1(inputPPMFileStruct.data, inputPPMFileStruct.width, inputPPMFileStruct.height, coeffA, coeffB, coeffC, tempVar1, tempVar2, b);
        
        for (size_t i = 0; i < sizeof(a); i++) {
            if (a[i] != b[i]) {
                printf("Not the same at: %li\n", i);
                free(tempVar1);
                free(tempVar2);
                free(denoisedRawData);
                free(inputPPMFileStruct.data);
                free(a);
                free(b);
                exit(1);
            }
        }
        printf("They are the same picture.\n");
        free(a);
        free(b);
    }
    

    // Free all the malloced space 
    free(tempVar1);
    free(tempVar2);
    free(denoisedRawData);
    free(inputPPMFileStruct.data);
    return 0;
}

void printUsage() {
    printf("Usage\n"
           "./denoise [-V<number>] [-B[<number>]] -c <float>,<float>,<float> -k -h -o <filename> <filename>\n\n"

           "OPTIONS:\n"
           "-V<number>, --version=<number>\n"
           "Choose which <version> of the implementation should be used (0 for the main implementation and 1 for the reference implementation).\n"
           "If this parameter is not set, version 0 is used by default.\n\n"

           "-B[<number>], --benchmark[=<number>]\n"
           "When set, the runtime of the implementation will be recorded. The optional <number> specifies the number of calls for which the runtime will be recorded.\n\n"

           "-c <float number>,<float number>,<float number>; --coeffs <float number>,<float number>,<float number>\n"
           "Use the coefficients defined in the command line, instead of the standard values\n\n"

           "-k, --correctness\n"
           "Compares the results between the two implementations.\n"

           "-h, --help\n"
           "Prints this help screen.\n\n"

            "-o <filename>, --output=<filename>\n"
           "Use the file named <filename> as an output file for the resulting image\n\n"
           
           "<filename>\n"
           "The name of the input file.\n"
           );
}