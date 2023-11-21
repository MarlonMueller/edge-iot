/**
 * @file main.c
 * @brief Entry point for ESP32 applicatio
 */
#include "esp_log.h"
#include "audio/preprocess.h"

#define MAIN_TAG "MAIN"

// Entry point for ESP32 application
#define TEST_WAV_SIZE 1000

static float audio[TEST_WAV_SIZE];

void app_main() {
    ESP_LOGI(MAIN_TAG, "Starting application");
    
    for (int i = 0; i < TEST_WAV_SIZE; i++) {
        audio[i] = 1.0;
    }
    
    float **mfcc_output;
    size_t mfcc_size = 0;

    malloc_mfcc_module();
    mfcc(audio, TEST_WAV_SIZE, &mfcc_output, &mfcc_size);
    free_mfcc_module();

    ESP_LOGI(MAIN_TAG, "MFCC num frames: %d", mfcc_size);

    
    if (mfcc_output == NULL) {
        ESP_LOGE(MAIN_TAG, "MFCC output is NULL");
    }

    for (int i = 0; i < mfcc_size; i++) {
        ESP_LOGI(MAIN_TAG, "Frame %d", i);

        for (int j = 0; j < 5; j++) {
            ESP_LOGI(MAIN_TAG, "MFCC %d: %f", j, mfcc_output[i][j]);
        }
    }

    // We free mfcc_output

    if (mfcc_output != NULL) {
        for (int i = 0; i < mfcc_size; ++i) {
            free(mfcc_output[i]);
        }

        free(mfcc_output);
    }

}