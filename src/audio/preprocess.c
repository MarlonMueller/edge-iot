/**
 * @file preprocess.c
 * @brief Audio preprocessing functions
 * 
 * This file contains the implementation of the audio preprocessing functions.
 * For PC compilation, RUN_PC directive must be defined.
*/
#include "audio/preprocess.h"

#include <math.h>

#ifndef RUN_PC

#include "esp_log.h"
#include "dsps_fft2r.h" // For FFT computation
#include "dsps_wind_hann.h" // For Hann windowing function
#include "dsps_dct.h" // For DCT computation

#else

// Change of ESP_LOGE, ESP_LOGI and ESP_LOGD to printf

#define ESP_LOGE(tag, format, ...) \
            printf("[%s] ERROR: " format "\n", tag, ##__VA_ARGS__)

#define ESP_LOGI(tag, format, ...) \
            printf("[%s]: " format "\n", tag, ##__VA_ARGS__)

// Definition of other functions needed

#include "sim_dsp/aux_err.h"
#include "sim_dsp/aux_fft.h"
#include "sim_dsp/aux_wind.h"
#include "sim_dsp/aux_dct.h"

#endif



static const char *PREPROCESS_TAG = "PREPROCESS";

// Constants for MFCC feature extraction. To be adjusted to our use case. 

#define SAMPLE_RATE 16000 // Hz
#define MAX_DB 80
#define FMIN 0
#define FMAX (SAMPLE_RATE / 2) // Nyquist (default by Librosa)

#define HOP_LENGTH_SAMPLES 90
#define WIN_LENGTH_SAMPLES 512     // for ESP-IDF implementation, 
#define NUM_FFT WIN_LENGTH_SAMPLES // must be Power of 2 because of FFT

#define N_MELS 5

#define HZ_TO_MEL(frequency) (2595.0 * log10(1.0 + (frequency) / 700.0))
#define MEL_TO_HZ(mels) (700.0 * (pow(10.0, (mels) / 2595.0) - 1.0))

// Next functions is based on Librosa implementation. Numerical stability

#define POWER_TO_DB(power) \
    (10.0 * log10(fmax(1e-10, power)) - 10.0 * log10(fmax(1e-10, 1.0)))


// The following variables must be global if their sizes are too big.
// Otherwise, stack overflow will occur.

static float *window;
static float *fft_operand;
static float *power_spectrum;
static float **mel_filt;
static float *mel_buffer;

// Another way to implement these variables, but memory can't be deallocated.

// static float window[WIN_LENGTH_SAMPLES];
// static float fft_operand[2*WIN_LENGTH_SAMPLES];
// static float power_spectrum[NUM_FFT/2 + 1];
// static float mel_filt[N_MELS][NUM_FFT / 2 + 1];
// static float mel_buffer[2*N_MELS];


// Auxiliary functions

size_t get_num_frames(size_t num_samples) {
    return (num_samples - WIN_LENGTH_SAMPLES) / HOP_LENGTH_SAMPLES;
}

void fft_frequencies(float *fft_freq) {
    float h = SAMPLE_RATE / (float) NUM_FFT;

    for (int i=0; i < NUM_FFT / 2 + 1; ++i) {
        fft_freq[i] = i * h;
    }
}

void mel_frequencies(float *mel_freq, int num_points) {
    float min_mel = HZ_TO_MEL(FMIN);
    float max_mel = HZ_TO_MEL(FMAX);
    float h = (max_mel - min_mel) / (num_points - 1);

    for (int i = 0; i < num_points; ++i) {
        mel_freq[i] = MEL_TO_HZ(min_mel + i * h);
    }
}

void mel_filters(float **weights) {
    float fft_freq[NUM_FFT / 2 + 1];
    fft_frequencies(fft_freq);

    float mel_freq[N_MELS + 2];
    mel_frequencies(mel_freq, N_MELS + 2);

    // Computation of mel filter bank

    for (size_t i=0; i < N_MELS; ++i) {
        for (size_t j=0; j < NUM_FFT/2 + 1; ++j) {
            float lower = mel_freq[i];
            float center = mel_freq[i + 1];
            float upper = mel_freq[i + 2];

            if (fft_freq[j] >= lower && fft_freq[j] <= upper) {
                if (fft_freq[j] < center) {
                    weights[i][j] = (fft_freq[j] - lower) / (center - lower);
                } else {
                    weights[i][j] = (upper - fft_freq[j]) / (upper - center);
                }
            } else {
                weights[i][j] = 0;
            }
        }
    }
};

// Main functions to interface. 

esp_err_t malloc_mfcc_module() {

    // Memory allocation

    window = (float *)malloc(WIN_LENGTH_SAMPLES * sizeof(float));
    fft_operand = (float *)malloc(2*WIN_LENGTH_SAMPLES * sizeof(float));
    power_spectrum = (float *)malloc((NUM_FFT/2 + 1) * sizeof(float));
    mel_buffer = (float *)malloc(2*N_MELS * sizeof(float));

    mel_filt = (float **)malloc(N_MELS * sizeof(float *));

    for (size_t i = 0; i < N_MELS; ++i) {
        mel_filt[i] = (float *)malloc((NUM_FFT/2 + 1) * sizeof(float));

        if (mel_filt[i] == NULL) {
            ESP_LOGE(PREPROCESS_TAG, "Error memory allocation");
            return ESP_ERR_NO_MEM;
        }
    }

    // Abort if any memory allocation fails.

    if (window == NULL || fft_operand == NULL || power_spectrum == NULL || 
       mel_filt == NULL || mel_buffer == NULL) {
        ESP_LOGE(PREPROCESS_TAG, "Error memory allocation");
        return ESP_ERR_NO_MEM;
    }

    // fft -> possible optimization: have it as a global variable

    dsps_fft2r_init_fc32(NULL, NUM_FFT);
    dsps_wind_hann_f32(window, WIN_LENGTH_SAMPLES); // Checked
    mel_filters(mel_filt);

    // Inform user

    ESP_LOGI(PREPROCESS_TAG, "MFCC module initialized");

    return ESP_OK;
}


esp_err_t mfcc(const float *wav_values, size_t num_samples, 
               float ***output, size_t *output_frames) {

    esp_err_t ret = ESP_OK;

    size_t num_frames = get_num_frames(num_samples);

    ESP_LOGI(PREPROCESS_TAG, "Number of samples: %d", num_samples);
    ESP_LOGI(PREPROCESS_TAG, "Number of frames: %d", num_frames);

    // Parameters check

    if (wav_values == NULL) {
        ESP_LOGE(PREPROCESS_TAG, "Error wav_values is NULL");
        *output_frames = 0;
        ret = ESP_ERR_INVALID_ARG;
        return ret;
    }
    
    *output = (float **)malloc(num_frames * sizeof(float *));

    if (*output == NULL) {
        ESP_LOGE(PREPROCESS_TAG, "Error output allocation");
        ret = ESP_ERR_NO_MEM;
        return ret;
    }
    
    for (size_t i = 0; i<num_frames; ++i) {
        (*output)[i] = (float *)malloc(N_MELS * sizeof(float));

        if ((*output)[i] == NULL) {
            ESP_LOGE(PREPROCESS_TAG, "Error output[%d] allocation", i);
            ret = ESP_ERR_NO_MEM;
            return ret;
        }
    }
    
    // Framing computation

    for (size_t i = 0; i<num_frames; ++i) {
        size_t ref_frame = i * HOP_LENGTH_SAMPLES;

        for (size_t j=0; j < WIN_LENGTH_SAMPLES; ++j) {
            fft_operand[2*j] = wav_values[ref_frame + j] * window[j]; // Re
            fft_operand[2*j + 1] = 0; // Im
        }

        ret = dsps_fft2r_fc32(fft_operand, WIN_LENGTH_SAMPLES);
        ret = dsps_bit_rev_fc32(fft_operand, WIN_LENGTH_SAMPLES);

        if (ret != ESP_OK) {
            ESP_LOGE(PREPROCESS_TAG, "Error FFT computation");
            return ret;
        }

        // Power spectrum computation

        // Notice that we only need the first half of the FFT result,
        // since the second half has the same values but in reverse order. 

        for (size_t j=0; j < NUM_FFT/2 + 1; ++j) {
            float re_part = fft_operand[2*j]; // Re
            float im_part = fft_operand[2*j + 1]; // Im

            float power = re_part * re_part + im_part * im_part;

            power_spectrum[j] = power; 
        }

        // Mel filter bank computation

        for (size_t j = 0; j < N_MELS; ++j) {
            float mel_power = 0;

            for (size_t k=0; k < NUM_FFT/2 + 1; ++k) {
                mel_power += power_spectrum[k] * mel_filt[j][k];
            }

            mel_buffer[j] = POWER_TO_DB(mel_power); // Re
        }

        for (size_t j = N_MELS; j < 2*N_MELS; ++j) {
            mel_buffer[j] = 0; // Im
        }

        // DCT computation. This output is different than Librosa's. 

        dsps_dct_f32(mel_buffer, N_MELS); 

        // Store result
        
        for (size_t j=0; j<N_MELS; ++j) {
            (*output)[i][j] = mel_buffer[j];
        }
        
    }

    *output_frames = num_frames; // Save number of frames within output_frames.

    // Free memory
    
    dsps_fft2r_deinit_fc32();
    
    return ret;
    
};


esp_err_t free_mfcc_module() {
    
    free(window);
    free(fft_operand);
    free(power_spectrum);
    free(mel_buffer);

    for (size_t i = 0; i < N_MELS; ++i) {
        free(mel_filt[i]);
    }

    free(mel_filt);

    // Inform user

    ESP_LOGI(PREPROCESS_TAG, "MFCC module freed");

    return ESP_OK;
}