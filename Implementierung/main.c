//
// Created by markus on 12/17/23.
//
#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "FileIO.h"
#include "ImageDenoiser.h"

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

    int opt;
    struct option long_options[] = {{"version",     optional_argument, NULL, 'V'},
                                    {"benchmark",   optional_argument, NULL, 'B'},
                                    {"input",     required_argument, NULL, 'i'},
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
    while ((opt = getopt_long(argc, argv, "V:B::i:o:c:::hk", long_options, NULL)) != -1) {
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
            // case 'B':
                // benchmarking = true;
                // if (OPTIONAL_ARGUMENT_IS_PRESENT) {
                //     iter = true;
                //     iterations = atoi(optarg);
                //     if (iterations < 0) {
                //         fprintf(stderr, "The number of calls needs to be > 0\n");
                //         printUsage();
                //         free(input);
                //         free(result);
                //         return WRONG_ARGUMENT_INPUT;
                //     }
                // }
                // break;
            case 'i':
                inputFilePath = optarg; // Get the input file path 
                if (!strcmp(inputFilePath, "")) {
                    fprintf(stderr, "Invalid Path!\n");
                    //exit(FAILED_ALLOCATION); TODO: define the errors
                }
                necessary--;
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

    // if (version == 0) {
    //     if (benchmarking) {
    //         if (!iter) {
    //             struct timespec start;
    //             clock_gettime(CLOCK_MONOTONIC, &start);
    //             denoise_V1(); //TODO: Define the parameters used in the function
    //             struct timespec end;
    //             clock_gettime(CLOCK_MONOTONIC, &end);
    //             double time = end.tv_sec - start.tv_sec + 1e-9 * (end.tv_nsec - start.tv_nsec);
    //             printf("done after %f seconds\n", time);
    //         } else {
    //             struct timespec start;
    //             clock_gettime(CLOCK_MONOTONIC, &start);
    //             for (int i = 1; i <= iterations; i++) {
    //                 denoise_V1();//TODO: Define the parameters used in the function
    //             }
    //             struct timespec end;
    //             clock_gettime(CLOCK_MONOTONIC, &end);
    //             double time = end.tv_sec - start.tv_sec + 1e-9 * (end.tv_nsec - start.tv_nsec);
    //             double average = time / iterations;
    //             printf("done after %f seconds on average (measured on %d iterations)\n", average, iterations);
    //         }
    //     } else {
    //         denoiose_V1();//TODO: Same here
    //     }
    // } else if (version == 1) {
    //     if (benchmarking) {
    //         if (!iter) {
    //             struct timespec start;
    //             clock_gettime(CLOCK_MONOTONIC, &start);
    //             denoise_V2();//TODO: Define the parameters used in the function
    //             struct timespec end;
    //             clock_gettime(CLOCK_MONOTONIC, &end);
    //             double time = end.tv_sec - start.tv_sec + 1e-9 * (end.tv_nsec - start.tv_nsec);
    //             printf("done after %f seconds\n", time);
    //         } else {
    //             struct timespec start;
    //             clock_gettime(CLOCK_MONOTONIC, &start);
    //             for (int i = 1; i <= iterations; i++) {
    //                 denoise_V2();//TODO: Same here
    //             }
    //             struct timespec end;
    //             clock_gettime(CLOCK_MONOTONIC, &end);
    //             double time = end.tv_sec - start.tv_sec + 1e-9 * (end.tv_nsec - start.tv_nsec);
    //             double average = time / iterations;
    //             printf("done after %f seconds on average (measured on %d iterations)\n", average, iterations);
    //         }
    //     } else {
    //         denoise_V2();//TODO: Same here
    //     }
    // } else {
    //     fprintf(stderr, "Please provide 0 or 1 as the implementation version\n");
    //     printUsage();
    //     exit(WRONG_ARGUMENT_INPUT);
    // }

    // if (correctness) {

    //     struct PPMFile *referenceResult = NULL; //TODO: just for logic purposes

    //     if (referenceResult == NULL) {
    //         fprintf(stderr, "Memory allocation failed\n");
    //         exit(FAILED_ALLOCATION);
    //     }

    //     if (version == 0) {
    //         denoise_V2(); //TODO: define the correct outputType
    //     } else {
    //         denoise_V1(); //TODO: define the correct outputType
    //     }
    //     if (equals(result, referenceResult)) {
    //         printf("The result for both implementations is the same\n");
    //     } else {
    //         printf("The results differ between the two implementations\n");
    //     }

    //     print_result_to_file(referenceResult, "reference"); //TODO: only for logic purposes(use the correct output method)

    //     freeAll(referenceResult);

    // }

    // Reading the PPM input file into a PPMFile structure 
    struct PPMFile inputPPMFileStruct = readPPMFile(inputFilePath);

    // Temporary variables 
    uint8_t* tempVar1;
    uint8_t* tempVar2;
    tempVar1 = (uint8_t*) malloc (inputPPMFileStruct.width * inputPPMFileStruct.height);
    tempVar2 = (uint8_t*) malloc (inputPPMFileStruct.width * inputPPMFileStruct.height);

    // The final output raw data 
    uint8_t* denoisedRawData = (uint8_t*) malloc (inputPPMFileStruct.width * inputPPMFileStruct.height);

    // Main denoise function version 0
    denoise(inputPPMFileStruct.data, inputPPMFileStruct.width, inputPPMFileStruct.height, coeffA, coeffB, coeffC, tempVar1, tempVar2, denoisedRawData);

    // write the denoised data back into a pgm file 
    writePGMFile(outputFilePath, denoisedRawData, inputPPMFileStruct.width, inputPPMFileStruct.height, inputPPMFileStruct.maxColorValue);

    // Free all the malloced space 
    free(tempVar1);
    free(tempVar2);
    free(denoisedRawData);
    return 0;
}

void printUsage() {
    printf("Usage\n"
           "./denoise [-V<number>] [-B[<number>]] -a <filename> -b <filename> -o <filename>\n\n"

           "OPTIONS:\n"
           "-V<number>, --version=<number>\n"
           "Choose which <version> of the implementation should be used (0 for the main implementation and 1 for the reference implementation).\n"
           "If this parameter is not set, version 0 is used by default.\n\n"

           "-B[<number>], --benchmark[=<number>]\n"
           "When set, the runtime of the implementation will be recorded. The optional <number> specifies the number of calls for which the runtime will be recorded.\n\n"
           
           "-i <filename>, --input=<filename>\n"
           "Use the file named <filename> as an input for the image.\n\n"

           "-o <filename>, --output=<filename>\n"
           "Use the file named <filename> as an output file for the resulting image\n\n"

           "-c <float number>,<float number>,<float number>; --coeffs <float number>,<float number>,<float number>\n"
           "Use the coefficients defined in the command line, instead of the standard values\n\n"

           "-h, --help\n"
           "Prints this help screen.\n\n"

           "-k, --correctness\n"
           "Compares the results between the two implementations."
           );
}