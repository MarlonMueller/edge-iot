/**
 * @file microphone.h
 * @brief Microphone acquire data.
 *
 * @details This file contains functions to record data from microphone SPH0645.
 */

#pragma once

#include <string.h>
#include "esp_err.h"

#define SD_PIN 4 //or DOUT
#define WS_PIN 5 //or LRCL
#define SCK_PIN 6 //or BCLK

/**
 * @brief Initialize I2S transmission for microphone.
 *
 * @param sample_rate Sample rate of the audio signal.
 */
void init_i2s_mic(uint32_t sample_rate);

/**
 * @brief Records audio from microphone.
 *
 * @param buffer Buffer to store the recorded data.
 * @param samples Number of samples to record.
 *
 * @pre Buffer must be allocated.
 */
esp_err_t read_i2s_mic(int16_t *buffer, size_t samples);

/**
 * @brief Uninstall I2S driver.
 */
void deinit_i2s_mic();