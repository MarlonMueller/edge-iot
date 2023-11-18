/**
 * @file preprocess.c
 * @brief Audio preprocessing functions
*/
#include "audio/preprocess.h"

#include "esp_dsp.h" // For ESP32 optimized audio processing functions

// Constants for MFCC feature extraction. To be adjusted to our use case. 

const int NUM_FFT = 512;
const int HOP_LENGTH = 30;
const int N_MELS = 40;

/**
 * @brief Read MFCC feature from file. Algorithm based on Librosa implementation:
 * https://librosa.org/doc/main/generated/librosa.feature.mfcc.html 
 * 
 * @param file_path [in] Path to MFCC feature file
 * @param output_mfcc [out] Output MFCC feature.
 * 
 * @return True if success, False if error
*/

int ReadMfcc(const char *file_path, float **output_mfcc) {
    return 0;
};
