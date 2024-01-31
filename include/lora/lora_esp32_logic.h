/**
 * @file lora_esp32_logic.h
 * 
 * @brief Contains the logic for the LoRa ESP32 module.
*/
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>


/**
 * @brief Set up the LoRa communication. 
 * 
 * @post LoRa module is physically set up. 
 * 
 * @note LoRa module is disconnected from ESP after return from Deep Sleep. 
 * This function needs to be called each time after wake up. 
 */
void setup_lora_comm();

/**
 * @brief Return the state of the connection with LoRa module.
 * 
 * @return True if the connection is initialized, false otherwise.
 */
bool is_initialized_comm();

/**
 * @brief Initialize communication protocol.
 */
void initialize_comm();

/**
 * @brief Send the detected classification of time frame to RPi.
 * 
 * @param timer_value Min until next package is expected.
 * 
 * @pre The LoRa communication module has been initialized. 
 * @post Detections in time frame are resetted after successful send. 
 */
void send_data(uint8_t *timer_value);

/**
 * @brief Activate status of a classification index. 
 * 
 * @param idx Index of the classification.
 * 
 * @note Values are preserved until next send_data() call.
 */
void set_activation(int idx);

#ifdef __cplusplus
}
#endif