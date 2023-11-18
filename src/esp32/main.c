/**
 * @file main.c
 * @brief Entry point for ESP32 applicatio
 */
#include "esp_log.h"

#include "audio/preprocess.h"

static const char *TAG = "main";

// Entry point for ESP32 application

void app_main() {
    // ReadMfcc(NULL, NULL);
    ESP_LOGI(TAG, "*** MFCC ***");
}