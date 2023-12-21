/**
 * @file preprocess.c
 * @brief Audio preprocessing functions
 * 
 * This file contains the implementation of the audio preprocessing functions.
 * For PC compilation, RUN_PC directive must be defined.
 * 
 * In order to run this code for Python, compile to obtain the .so file with
 * the following command:
 * 
 * gcc -x c -o src/audio/preprocess.so -D RUN_PC -shared -fPIC -I include/ src/audio/preprocess.cpp -lm
 * 
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

#define HOP_LENGTH_SAMPLES 256
#define WIN_LENGTH_SAMPLES 1024     // for ESP-IDF implementation, 
#define NUM_FFT WIN_LENGTH_SAMPLES // must be Power of 2 because of FFT

#define N_MELS 40
#define N_MFCC 32  // NOTE: Must be less than N_MELS

#define HZ_TO_MEL(frequency) (2595.0 * log10(1.0 + (frequency) / 700.0))
#define MEL_TO_HZ(mels) (700.0 * (pow(10.0, (mels) / 2595.0) - 1.0))

// Next functions is based on Librosa implementation. Numerical stability

#define POWER_TO_DB(power) \
    (10.0 * log10(fmax(1e-10, power)) - 10.0 * log10(fmax(1e-10, 1.0)))


// The following variables must be global if their sizes are too big.
// Otherwise, stack overflow will occur.

static float *s_window;
static float *s_fft_operand;
static float *s_power_spectrum;
static float **s_mel_filt;
static float *s_mel_buffer;

// Another way to implement these variables, but memory can't be deallocated.

// static float s_window[WIN_LENGTH_SAMPLES];
// static float s_fft_operand[2*WIN_LENGTH_SAMPLES];
// static float s_power_spectrum[NUM_FFT/2 + 1];
// static float s_mel_filt[N_MELS][NUM_FFT / 2 + 1];
// static float s_mel_buffer[2*N_MELS];

extern "C" {

// Auxiliary functions

size_t get_num_mfcc() 
{
    return N_MFCC;
}

size_t get_num_frames(size_t num_samples) 
{
    return (num_samples - WIN_LENGTH_SAMPLES) / HOP_LENGTH_SAMPLES;
}

void fft_frequencies(float *fft_freq) 
{
    float h = SAMPLE_RATE / (float) NUM_FFT;

    for (int i=0; i < NUM_FFT / 2 + 1; ++i) {
        fft_freq[i] = i * h;
    }
}

void mel_frequencies(double *mel_freq, int num_points) 
{
    double min_mel = HZ_TO_MEL(FMIN);
    double max_mel = HZ_TO_MEL(FMAX);

    for (int i = 0; i < num_points; ++i) {
        mel_freq[i] = MEL_TO_HZ(min_mel + i * (max_mel - min_mel) / (num_points - 1));
    }
}

void mel_filters(float **weights) 
{
    float fft_freq[NUM_FFT / 2 + 1]; // Checked
    fft_frequencies(fft_freq);

    double mel_freq[N_MELS + 2];
    mel_frequencies(mel_freq, N_MELS + 2);

    // Computation of mel filter bank

    for (size_t i=0; i < N_MELS; ++i) {
        for (size_t j=0; j < NUM_FFT/2 + 1; ++j) {
            double lower = mel_freq[i];
            double center = mel_freq[i + 1];
            double upper = mel_freq[i + 2];

            if (fft_freq[j] >= lower && fft_freq[j] <= upper) {
                if (fft_freq[j] < center) {
                    weights[i][j] = (fft_freq[j] - lower) / (center - lower);
                } else {
                    weights[i][j] = (upper - fft_freq[j]) / (upper - center);
                }
            } else {
                weights[i][j] = 0;
            }

            // Slaney normalization

            weights[i][j] *= 2.0 / (mel_freq[i + 2] - mel_freq[i]);
        }
    }
};

void preemphasis(float *wav_values, size_t num_samples, float coeff) 
{
    for (size_t i = num_samples - 1; i > 0; --i) {
        wav_values[i] -= coeff * wav_values[i - 1];
    }
}

// Main functions to interface. 

esp_err_t malloc_mfcc_module() 
{
    // Memory allocation

    s_window = (float *)malloc(WIN_LENGTH_SAMPLES * sizeof(float));
    s_fft_operand = (float *)malloc(2*WIN_LENGTH_SAMPLES * sizeof(float));
    s_power_spectrum = (float *)malloc((NUM_FFT/2 + 1) * sizeof(float));
    s_mel_buffer = (float *)malloc(2*N_MELS * sizeof(float));

    s_mel_filt = (float **)malloc(N_MELS * sizeof(float *));

    for (size_t i = 0; i < N_MELS; ++i) {
        s_mel_filt[i] = (float *)malloc((NUM_FFT/2 + 1) * sizeof(float));

        if (s_mel_filt[i] == NULL) {
            ESP_LOGE(PREPROCESS_TAG, "Error memory allocation");
            return ESP_ERR_NO_MEM;
        }
    }

    // Abort if any memory allocation fails.

    if (s_window == NULL || s_fft_operand == NULL || s_power_spectrum == NULL || 
       s_mel_filt == NULL || s_mel_buffer == NULL) {
        ESP_LOGE(PREPROCESS_TAG, "Error memory allocation");
        return ESP_ERR_NO_MEM;
    }

    // fft -> possible optimization: have it as a global variable

    dsps_fft2r_init_fc32(NULL, NUM_FFT);
    dsps_wind_hann_f32(s_window, WIN_LENGTH_SAMPLES); // Checked
    mel_filters(s_mel_filt);

    // Inform user

    ESP_LOGI(PREPROCESS_TAG, "MFCC module initialized");

    return ESP_OK;
}


esp_err_t mfcc(int8_t *wav_values, size_t num_samples, 
               float ***output, size_t *output_frames) 
{

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
        (*output)[i] = (float *)malloc(N_MFCC * sizeof(float));

        if ((*output)[i] == NULL) {
            ESP_LOGE(PREPROCESS_TAG, "Error output[%d] allocation", i);
            ret = ESP_ERR_NO_MEM;
            return ret;
        }
    }

    // Preemphasis

    // preemphasis(wav_values, num_samples, 0.97); // 0.97 usual value
    
    // Framing computation

    for (size_t i = 0; i<num_frames; ++i) {
        size_t ref_frame = i * HOP_LENGTH_SAMPLES;

        for (size_t j=0; j < WIN_LENGTH_SAMPLES; ++j) {
            float value = wav_values[ref_frame + j] / (float) INT8_MAX;
            // float value = wav_values[ref_frame + j];
            s_fft_operand[2*j] = value * s_window[j]; // Re
            s_fft_operand[2*j + 1] = 0; // Im
        }

        ret = dsps_fft2r_fc32(s_fft_operand, WIN_LENGTH_SAMPLES);
        ret = dsps_bit_rev_fc32(s_fft_operand, WIN_LENGTH_SAMPLES);

        if (ret != ESP_OK) {
            ESP_LOGE(PREPROCESS_TAG, "Error FFT computation");
            return ret;
        }

        // Power spectrum computation

        // Notice that we only need the first half of the FFT result,
        // since the second half has the same values but in reverse order. 

        for (size_t j=0; j < NUM_FFT/2 + 1; ++j) {
            float re_part = s_fft_operand[2*j]; // Re
            float im_part = s_fft_operand[2*j + 1]; // Im

            float power = re_part * re_part + im_part * im_part;

            s_power_spectrum[j] = power; 
        }

        // Mel filter bank computation

        for (size_t j = 0; j < N_MELS; ++j) {
            double mel_power = 0;

            for (size_t k=0; k < NUM_FFT/2 + 1; ++k) {
                mel_power += s_power_spectrum[k] * s_mel_filt[j][k]; // Checked
            }

            s_mel_buffer[j] = POWER_TO_DB(mel_power); // Re
        }

        for (size_t j = N_MELS; j < 2*N_MELS; ++j) {
            s_mel_buffer[j] = 0; // Im
        }

        // DCT computation. This output is different than Librosa's. 
        float dct_output[N_MELS];
        dsps_dct_f32_ref(s_mel_buffer, N_MELS, dct_output); 

        // Store result
        
        // The first coefficient is discarded, as in Librosa's implementation.

        for (size_t j=0; j<N_MFCC; ++j) {
            (*output)[i][j] = dct_output[j+1];
        }
        
    }

    *output_frames = num_frames; // Save number of frames within output_frames.

    // Free memory
    
    dsps_fft2r_deinit_fc32();
    
    return ret;
    
};


esp_err_t free_mfcc_module() 
{
    free(s_window);
    free(s_fft_operand);
    free(s_power_spectrum);
    free(s_mel_buffer);

    for (size_t i = 0; i < N_MELS; ++i) {
        free(s_mel_filt[i]);
    }

    free(s_mel_filt);

    // Inform user

    ESP_LOGI(PREPROCESS_TAG, "MFCC module freed");

    return ESP_OK;
}

}