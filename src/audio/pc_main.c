/**
 * g++ -g -D RUN_PC -I include/ src/audio/pc_main.c src/audio/preprocess.cpp -o build/pc_test
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "audio/preprocess.h"

#define TEST_WAV_SIZE 48000

float audio[TEST_WAV_SIZE];

int main() {
    printf("Starting application");
    
    float **mfcc_output;
    size_t mfcc_size = 0;

    for (int i = 0; i < TEST_WAV_SIZE; i++) {
        audio[i] = 1.0f;
    }

    malloc_mfcc_module();
    mfcc(audio, TEST_WAV_SIZE, &mfcc_output, &mfcc_size);
    free_mfcc_module();

    printf("MFCC num frames: %d\n", mfcc_size);

    
    if (mfcc_output == NULL) {
        printf("MFCC output is NULL\n");
    }

    for (int i = 0; i < mfcc_size; i++) {
        printf("Frame %d\n", i);

        for (int j = 0; j < 5; j++) {
            printf("MFCC %d: %f\n", j, mfcc_output[i][j]);
        }
    }

    // We free mfcc_output

    if (mfcc_output != NULL) {
        for (int i = 0; i < mfcc_size; ++i) {
            free(mfcc_output[i]);
        }

        free(mfcc_output);
    }

    return 0;
}