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

#define NUM_TRIES 3 // number of tries to send a packet
#define PETITION_DELAY (100 / portTICK_PERIOD_MS) // ms between petitions
#define NEXT_INIT_RETRY (300 / portTICK_PERIOD_MS) // ms between init retries
#define NEXT_PERIODIC_DATA (500 / portTICK_PERIOD_MS) // ms between periodic data

typedef struct {
    bool d1, d2, d3;
} detection_t;

static uint8_t local_id;
static uint8_t counter;
static detection_t detection;

static uint8_t rx_buffer[256];
static uint8_t tx_buffer[256];

void setup_lora_comm();
void initialize_comm();
void send_periodic_data(void *pvParameters);
void set_activation(int idx);


#ifdef __cplusplus
}
#endif