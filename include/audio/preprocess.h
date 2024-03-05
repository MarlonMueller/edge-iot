/**
 * @file preprocess.h
 * @brief Audio preprocessing functions
 * 
 * @details This file contains functions for bird sounds preprocessing for ESP32. 
*/

#pragma once

#ifndef RUN_PC
#include "esp_err.h"
#else
#include <stdint.h>
#include "sim_dsp/aux_err.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get number of MFCC features.
 * 
 * @return Number of MFCC features.
*/
size_t get_num_mfcc();

/**
 * @brief Initialization of MFCC preprocessing module. Memory sizes allocated.
 * 
 * @return Error values according to ESP-IDF coding style.
*/
esp_err_t malloc_mfcc_module();

/**
 * @brief Calculate MFCC. Algorithm based on Librosa implementation:
 * https://librosa.org/doc/main/generated/librosa.feature.mfcc.html 
 * 
 * Note: Unlike to Librosa, if a certain window size exceed the length 
 * of the signal, the function will not pad the signal with zeros. Final
 * data will be lost. 
 * 
 * @param wav_values [in] Input audio signal.
 * @param num_samples [in] Size of the input audio signal.
 * @param output_mfcc [out] Output MFCC feature.
 * @param output_frames [out] Number of frames in the output MFCC feature.
 * 
 * 
 * @pre output_mfcc must not be allocated before calling this function.
 * Otherwise, memory leak will occur.
 * 
 * @return Error values according to ESP-IDF coding style.
*/
esp_err_t mfcc(int16_t *wav_values, size_t num_samples, 
               float ***output, size_t *output_frames);


/**
 * @brief Free memory allocated for MFCC preprocessing module.
 * 
 * @return ESP_OK if success, otherwise undetermined.
*/
esp_err_t free_mfcc_module();


#ifdef __cplusplus
}
#endif