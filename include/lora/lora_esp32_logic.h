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

void setup_lora_comm();
bool is_initialized_comm();
void initialize_comm();
void send_data();
void set_activation(int idx);

#ifdef __cplusplus
}
#endif