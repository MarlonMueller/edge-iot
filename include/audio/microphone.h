/**
 * @file microphone.h
 * @brief Microphone acquire data. 
 * 
 * @details This file contains functions to record data from microphone SPH0645. 
*/

#pragma once

// PIN LAYOUT: 
// The following pins are used. Remember to also connect the power supply and
// ground pins. 5 pins are used in total. 

#define DATA_PIN 7
#define BCLK_PIN 4
#define WS_PIN 17

/**
 * @brief Initialize I2S transmission for microphone. 
*/
void init_i2s_mic();

/**
 * @brief Records audio from microphone.
 * 
 * @param buffer Buffer where data shall be stored.
 * @param num_values Number of elements to be read. 
 * 
 * @pre buffer is allocated with the number of elements indicated by num_values. 
*/
void record_i2s_mic(float *buffer, int num_values);

/**
 * @brief Deinitialize I2S transmission for microphone. 
*/
void deinit_i2s_mic();